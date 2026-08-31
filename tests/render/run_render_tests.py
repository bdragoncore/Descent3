#!/usr/bin/env python3
"""
Render Test Runner - Python3 master script for d3src/tests/render.

Builds and runs all render tests (single executable d3_render_tests, which
contains every suite under tests/render), then generates:
  - Call graph (callgraph.json when tracing enabled)
  - HTML report (render_report.html) and per-trace HTML when tracing

Usage:
    ./run_render_tests.py [--build-dir DIR] [--output-dir DIR] [--report NAME]
                         [--baseline-dir DIR] [--verbose] [--rebuild] [--no-build]
                         [--tracing] [--update-baseline]

Options:
    --build-dir DIR       CMake build directory (default: build)
    --output-dir DIR      Output directory for PNGs/HTML/traces (default: build_dir/tests/render_output)
    --baseline-dir DIR    Directory containing reference MD5 files (default: tests/render_output_baseline)
    --report NAME         HTML report filename (default: render_report.html)
    --verbose             Verbose test output
    --rebuild             Force rebuild of test binaries
    --no-build            Skip build, assume binaries exist
    --tracing             Enable function instrumentation (auto-detected by default)
    --update-baseline     Overwrite baseline MD5s with current run
    --serve               Start Next.js call graph viewer after run
    --port N              Port for Next.js viewer (default: 3000)
"""

import argparse
import json
import os
import re
import shutil
import subprocess
import sys
import time
from dataclasses import dataclass
from datetime import datetime
from pathlib import Path
from glob import glob

# Add tests/ to path for report_tools (script lives in tests/render/)
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from report_tools.html_report import generate as generate_html_report


# Test executables and their required environment
RENDER_TESTS = [
    {
        "executable": "d3_render_tests",
        "environment": {"SDL_VIDEODRIVER": "offscreen"},
        "description": "SDL offscreen rendering",
    },
]

# Paths to callgrind tooling
_REPORTTOOLS_DIR = os.path.join(os.path.dirname(__file__), "..", "report_tools")
_PARSE_CALLGRIND = os.path.join(_REPORTTOOLS_DIR, "parse_callgrind.py")
_VIEWER_TEMPLATE = os.path.join(_REPORTTOOLS_DIR, "callgraph_viewer_template.html")


@dataclass
class ParsedTestResult:
    """Parsed result from a single test case."""

    name: str
    suite: str
    status: str  # PASSED/FAILED
    duration_ms: int


@dataclass
class TestExecResult:
    """Result from running a test executable."""

    exe: str
    passed: bool
    pngs: list
    md5s: list
    traces: list
    environment: dict
    test_results: list


def parse_args():
    parser = argparse.ArgumentParser(
        description="Run render tests and generate HTML report",
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    parser.add_argument(
        "--build-dir", default="build", help="CMake build directory (default: build)"
    )
    parser.add_argument(
        "--output-dir",
        default=None,
        help="Output directory for PNGs/HTML/traces (default: build_dir/tests/render_output)",
    )
    parser.add_argument(
        "--baseline-dir",
        default=None,
        help="Directory containing reference MD5 files (default: tests/render_output_baseline)",
    )
    parser.add_argument(
        "--report",
        default="render_report.html",
        help="HTML report filename (default: render_report.html)",
    )
    parser.add_argument(
        "--verbose", "-v", action="store_true", help="Verbose test output"
    )
    parser.add_argument(
        "--rebuild", action="store_true", help="Force rebuild of test binaries"
    )
    parser.add_argument(
        "--no-build", action="store_true", help="Skip build, assume binaries exist"
    )
    parser.add_argument(
        "--tracing",
        action="store_true",
        default=None,
        help="Enable function instrumentation (default: auto-detected)",
    )
    parser.add_argument(
        "--update-baseline",
        action="store_true",
        help="Overwrite baseline MD5s with current run",
    )
    parser.add_argument(
        "--serve",
        action="store_true",
        help="Start Next.js call graph viewer after run",
    )
    parser.add_argument(
        "--port",
        type=int,
        default=3000,
        help="Port for Next.js viewer (default: 3000)",
    )
    return parser.parse_args()


def detect_tracing_available(build_dir: str) -> bool:
    """Detect if tracing is available (clang + tracer library)."""
    # Check for clang compiler
    cache_file = Path(build_dir) / "CMakeCache.txt"
    if not cache_file.exists():
        return False

    with open(cache_file) as f:
        content = f.read()
        # Check for clang (either as FILEPATH or STRING)
        if "clang" not in content.lower():
            return False

    # Check for tracer library
    tracer_lib = Path(build_dir) / "tests" / "libtracer.a"
    return tracer_lib.exists()


def ensure_cmake_configured(build_dir: str, tests_dir: str) -> bool:
    """Ensure cmake is configured."""
    cache_file = Path(build_dir) / "CMakeCache.txt"
    if cache_file.exists():
        return True

    print(f"Configuring CMake in {build_dir}...")
    # Run cmake from project root (parent of tests_dir)
    project_root = Path(tests_dir).parent
    result = subprocess.run(
        ["cmake", "-B", build_dir, "-DBUILD_TESTING=ON"],
        cwd=str(project_root),
        capture_output=True,
        text=True,
    )
    if result.returncode != 0:
        print(f"CMake configuration failed:\n{result.stderr}", file=sys.stderr)
        return False
    return True


def ensure_built(build_dir: str, target: str, force: bool = False) -> bool:
    """Ensure a specific target is built."""
    binary_path = Path(build_dir) / "tests" / target

    if not force and binary_path.exists():
        return True

    print(f"Building {target}...")
    result = subprocess.run(
        ["cmake", "--build", build_dir, "--target", target],
        capture_output=True,
        text=True,
    )
    if result.returncode != 0:
        print(f"Build failed for {target}:\n{result.stderr}", file=sys.stderr)
        return False
    return True


def run_test_executable(
    executable: str,
    build_dir: str,
    environment: dict,
    output_dir: str,
    verbose: bool = False,
    tracing: bool = False,
) -> TestExecResult:
    """Run a test executable and return output, duration, and parsed results."""
    binary_path = Path(build_dir) / "tests" / executable

    if not binary_path.exists():
        return TestExecResult(
            exe=executable,
            passed=False,
            pngs=[],
            md5s=[],
            traces=[],
            environment=environment,
            test_results=[],
        )

    # Build environment - pass output dir as env var for tracing
    env = os.environ.copy()
    env.update(environment)

    if tracing:
        # Set TRACE_OUTPUT_DIR for tracer
        env["TRACE_OUTPUT_DIR"] = os.path.abspath(output_dir)
        # Force eager symbol binding so dladdr() sees resolved symbols for tracing
        env["LD_BIND_NOW"] = "1"

    # Run the test - don't use --gtest_brief to get per-test output
    cmd = [str(binary_path)]

    start_time = time.time()
    result = subprocess.run(
        cmd,
        capture_output=True,
        text=True,
        env=env,
        cwd=output_dir,  # Run from output dir so trace files are written there
    )
    duration_ms = int((time.time() - start_time) * 1000)

    output = result.stdout + result.stderr

    # Parse test results
    tests = parse_gtest_output(output)

    # Collect output files
    pngs = glob(os.path.join(output_dir, "Test*.png"))
    md5s = glob(os.path.join(output_dir, "Test*.md5"))
    traces = glob(os.path.join(output_dir, "trace__*.json")) if tracing else []

    return TestExecResult(
        exe=executable,
        passed=(result.returncode == 0),
        pngs=pngs,
        md5s=md5s,
        traces=traces,
        environment=environment,
        test_results=tests,
    )


def parse_gtest_output(output: str) -> list[ParsedTestResult]:
    """Parse Google Test output to extract test results."""
    results = []

    # Match lines like: [       OK ] D3RenderTest.RenderSingleColoredQuad (91 ms)
    pattern = r"\[\s+OK\s+\]\s+(\w+)\.(\w+)\s+\(([\d]+)\s+ms\)"

    for match in re.finditer(pattern, output):
        suite = match.group(1)
        name = match.group(2)
        duration = int(match.group(3))

        results.append(
            ParsedTestResult(
                name=name, suite=suite, status="PASSED", duration_ms=duration
            )
        )

    # Also match FAILED tests: [  FAILED  ] TestName.TestCase (123 ms)
    pattern_failed = r"\[\s+FAILED\s+\]\s+(\w+)\.(\w+)\s+\(([\d]+)\s+ms\)"

    for match in re.finditer(pattern_failed, output):
        suite = match.group(1)
        name = match.group(2)
        duration = int(match.group(3))

        results.append(
            ParsedTestResult(
                name=name, suite=suite, status="FAILED", duration_ms=duration
            )
        )

    return results


def start_viewer(output_dir: str, port: int = 3000) -> None:
    """Start the Next.js call graph viewer with OUTPUT_DIR set to output_dir."""
    viewer_dir = Path(__file__).resolve().parent.parent / "callgraph-viewer"
    if not (viewer_dir / "package.json").exists():
        print("[viewer] callgraph-viewer not found, skipping")
        return
    env = os.environ.copy()
    env["OUTPUT_DIR"] = os.path.abspath(output_dir)
    try:
        subprocess.Popen(
            ["npm", "run", "dev", "--", "-p", str(port)],
            cwd=str(viewer_dir),
            env=env,
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
        )
        print(f"[viewer] Starting at http://localhost:{port}")
        import webbrowser
        time.sleep(2)
        webbrowser.open(f"http://localhost:{port}")
    except Exception as e:
        print(f"[viewer] Failed to start: {e}")


def check_valgrind_available() -> bool:
    try:
        r = subprocess.run(
            ["valgrind", "--version"],
            capture_output=True,
            text=True,
        )
        return r.returncode == 0
    except FileNotFoundError:
        return False


def run_with_callgrind(exe, gtest_filter, toggle_fn, root_fn, output_dir, verbose=False):
    safe = gtest_filter.replace(".", "__").replace("/", "_")
    cg_out = os.path.join(output_dir, f"{safe}__callgrind.out")
    graph_js = os.path.join(output_dir, f"{safe}__graph.json")
    html_out = os.path.join(output_dir, f"{safe}__callgraph.html")

    cmd = [
        "valgrind",
        "--tool=callgrind",
        f"--callgrind-out-file={cg_out}",
        "--collect-atstart=no",
        "--instr-atstart=no",
        f"--toggle-collect={toggle_fn}",
        exe,
        f"--gtest_filter={gtest_filter}",
    ]
    print(f"  [callgrind] {gtest_filter}")
    subprocess.run(
        cmd,
        capture_output=not verbose,
        text=True,
        cwd=output_dir,
        env={**os.environ, "SDL_VIDEODRIVER": "offscreen"},
    )

    if not os.path.exists(cg_out):
        print(f"  [callgrind] ERROR: no output at {cg_out}")
        return None

    r = subprocess.run(
        [sys.executable, _PARSE_CALLGRIND, cg_out, graph_js, root_fn],
        capture_output=not verbose,
        text=True,
    )
    if r.returncode != 0 or not os.path.exists(graph_js):
        print(f"  [callgrind] parse failed")
        return None

    with open(_VIEWER_TEMPLATE) as f:
        tmpl = f.read()
    with open(graph_js) as f:
        gdata = f.read()

    # Must match callgraph_viewer_template.html fetch block exactly (4 spaces before .then)
    FETCH_BLOCK = (
        '  fetch("graph.json")\n'
        "    .then(r => {\n"
        '        if (!r.ok) throw new Error("graph.json not found");\n'
        "        return r.json();\n"
        "    })\n"
        "    .then(graph => {\n"
        "        init(graph);\n"
        "    })\n"
        "    .catch(err => {\n"
        '        document.getElementById("stats").textContent =\n'
        '            "Error loading graph.json: " + err.message;\n'
        "    });"
    )
    inline = f"const GRAPH_DATA = {gdata};\n    init(GRAPH_DATA);"
    html = tmpl.replace(FETCH_BLOCK, inline)
    with open(html_out, "w") as f:
        f.write(html)
    print(f"  [callgrind] -> {html_out}")
    return html_out


def main():
    args = parse_args()

    # Resolve paths (script lives in tests/render/, so tests_dir is parent)
    script_dir = Path(os.path.dirname(os.path.abspath(__file__)))
    tests_dir = script_dir.parent
    # Resolve build_dir relative to project root (parent of tests_dir)
    project_root = tests_dir.parent
    build_dir = str(project_root / args.build_dir)

    if args.output_dir:
        output_dir = args.output_dir
    else:
        # Match CMake RENDER_OUTPUT_DIR so we collect PNGs where the test binary writes
        output_dir = str(Path(build_dir) / "tests" / "render_output")

    if args.baseline_dir:
        baseline_dir = args.baseline_dir
    else:
        baseline_dir = str(tests_dir / "render_output_baseline")

    # Ensure output directory exists
    os.makedirs(output_dir, exist_ok=True)

    # Detect tracing availability
    tracing_available = detect_tracing_available(build_dir)
    tracing_enabled = args.tracing if args.tracing is not None else tracing_available

    if args.tracing and not tracing_available:
        print(
            "Warning: --tracing specified but tracing not available (no clang or tracer library)"
        )

    print("=" * 60)
    print("Render Test Runner")
    print("=" * 60)
    print(f"Build directory: {build_dir}")
    print(f"Output directory: {output_dir}")
    print(f"Baseline directory: {baseline_dir}")
    print(f"Tracing: {'enabled' if tracing_enabled else 'disabled'}")
    callgrind_available = check_valgrind_available()
    print(f"Callgrind: {'enabled' if callgrind_available else 'disabled'}")
    print()

    # Build tests if needed
    if not args.no_build:
        # Configure cmake if needed
        if not ensure_cmake_configured(build_dir, str(tests_dir)):
            print("ERROR: CMake configuration failed", file=sys.stderr)
            return 1

        # Build each test executable
        for test in RENDER_TESTS:
            if not ensure_built(build_dir, test["executable"], args.rebuild):
                print(f"ERROR: Failed to build {test['executable']}", file=sys.stderr)
                return 1

    # Run tests
    all_results = []
    total_passed = 0
    total_failed = 0

    for test in RENDER_TESTS:
        executable = test["executable"]
        env = test["environment"]

        print(f"Running {executable} ({test['description']})...")

        exec_result = run_test_executable(
            executable, build_dir, env, output_dir, args.verbose, tracing_enabled
        )

        if exec_result.test_results:
            print(f"  {len(exec_result.test_results)} tests found")

            for test_result in exec_result.test_results:
                # Build result dict for html_report
                result_dict = {
                    "exe": executable,
                    "test_name": test_result.name,
                    "test_suite": test_result.suite,
                    "passed": test_result.status == "PASSED",
                    "duration_ms": test_result.duration_ms,
                    "pngs": exec_result.pngs,
                    "md5s": exec_result.md5s,
                    "traces": exec_result.traces,
                    "environment": env,
                    "callgraphs": [],
                }
                all_results.append(result_dict)

                if test_result.status == "PASSED":
                    total_passed += 1
                    print(
                        f"    ✓ {test_result.suite}.{test_result.name} ({test_result.duration_ms}ms)"
                    )
                else:
                    total_failed += 1
                    print(
                        f"    ✗ {test_result.suite}.{test_result.name} ({test_result.duration_ms}ms)"
                    )
        else:
            print(f"  No tests found or execution failed")
            if args.verbose:
                print(f"  Output: {exec_result}")

        print()

    # Callgrind call graph generation
    abs_output = os.path.abspath(output_dir)
    exe_path = os.path.join(build_dir, "tests", "d3_render_tests")
    if check_valgrind_available():
        for res in all_results:
            # Valgrind needs exact demangled name; GTest uses Suite_Test_Test::TestBody()
            toggle_fn = f"{res['test_suite']}_{res['test_name']}_Test::TestBody()"
            # Parser gets friendly root for graph.json "root" and substring-matches to find BFS root
            root_fn = f"{res['test_suite']}::{res['test_name']}"
            cg_html = run_with_callgrind(
                exe=exe_path,
                gtest_filter=f"{res['test_suite']}.{res['test_name']}",
                toggle_fn=toggle_fn,
                root_fn=root_fn,
                output_dir=abs_output,
                verbose=args.verbose,
            )
            if cg_html:
                res["callgraphs"].append(cg_html)
    else:
        print("[callgrind] valgrind not found, skipping call graphs")

    # Collect trace JSON paths once for DB, call graph, and per-test HTML
    all_trace_paths = []
    if tracing_enabled:
        all_trace_paths = glob(os.path.join(output_dir, "trace__*.json"))

    # Build callgraph when tracing is enabled (even if no trace files this run)
    callgraph_path = None
    if tracing_enabled:
        from report_tools.callgraph import build_call_graph

        callgraph = build_call_graph(all_trace_paths) if all_trace_paths else {}
        callgraph_path = os.path.join(build_dir, "callgraph.json")
        with open(callgraph_path, "w") as f:
            json.dump(callgraph, f, indent=2)
        print(f"  Call graph: {callgraph_path}")

    # Generate trace HTML files
    if tracing_enabled and all_trace_paths:
        print("Generating trace HTML files...")
        for trace_json in all_trace_paths:
            try:
                from report_tools.generate_html import generate_html

                html_output = trace_json.replace(".json", ".html")
                with open(html_output, "w") as f:
                    f.write(generate_html(trace_json))
                print(f"  Generated: {os.path.basename(html_output)}")
            except Exception as e:
                print(f"  Error generating HTML for {trace_json}: {e}")
        print()

    # Generate HTML report
    viewer_base_url = f"http://localhost:{args.port}" if args.serve else None
    print("Generating HTML report...")
    try:
        generate_html_report(
            results=all_results,
            output_dir=output_dir,
            baseline_dir=baseline_dir if os.path.exists(baseline_dir) else None,
            report_path=args.report,
            update_baseline=args.update_baseline,
            tracing_enabled=tracing_enabled,
            callgraph_path=callgraph_path,
            viewer_base_url=viewer_base_url,
        )
        print(f"  Report saved to: {os.path.join(output_dir, args.report)}")
    except Exception as e:
        print(f"  Error generating report: {e}")
        import traceback

        traceback.print_exc()

    if args.serve:
        start_viewer(abs_output, args.port)

    # Print summary and where to find output
    print()
    print("=" * 60)
    print("Summary")
    print("=" * 60)
    print(f"Total tests:  {total_passed + total_failed}")
    print(f"Passed:       {total_passed}")
    print(f"Failed:       {total_failed}")
    print()
    print("Output is under the build tree (not tests/render_output):")
    print(f"  {os.path.abspath(output_dir)}")
    print(f"  Report: {os.path.join(output_dir, args.report)}")
    print()

    return 0 if total_failed == 0 else 1


if __name__ == "__main__":
    sys.exit(main())
