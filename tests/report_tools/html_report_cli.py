#!/usr/bin/env python3
"""
CLI wrapper for report_tools.html_report.generate.

Usage:
  python3 tests/report_tools/html_report_cli.py \\
      RESULTS_JSON OUTPUT_DIR BASELINE_DIR_OR_NONE REPORT_FILENAME \\
      UPDATE_BASELINE(0|1) TRACING_ENABLED(0|1) CALLGRAPH_PATH_OR_NONE \\
      VIEWER_BASE_URL_OR_NONE
"""

import json
import os
import sys


def _add_tests_to_sys_path() -> None:
    here = os.path.dirname(os.path.abspath(__file__))
    tests_dir = os.path.dirname(here)
    if tests_dir not in sys.path:
        sys.path.insert(0, tests_dir)


def main(argv: list[str]) -> int:
    if len(argv) < 9:
        print(
            "Usage: html_report_cli.py RESULTS_JSON OUTPUT_DIR "
            "BASELINE_DIR_OR_NONE REPORT_FILENAME "
            "UPDATE_BASELINE(0|1) TRACING_ENABLED(0|1) "
            "CALLGRAPH_PATH_OR_NONE VIEWER_BASE_URL_OR_NONE",
            file=sys.stderr,
        )
        return 1

    results_json = argv[1]
    output_dir = argv[2]
    baseline_dir_arg = argv[3]
    report_filename = argv[4]
    update_baseline = argv[5] == "1"
    tracing_enabled = argv[6] == "1"
    callgraph_path_arg = argv[7]
    viewer_base_url_arg = argv[8]

    baseline_dir = None if baseline_dir_arg == "NONE" else baseline_dir_arg
    callgraph_path = None if callgraph_path_arg == "NONE" else callgraph_path_arg
    viewer_base_url = (
        None if viewer_base_url_arg == "NONE" else viewer_base_url_arg
    )

    with open(results_json) as f:
        results = json.load(f)

    _add_tests_to_sys_path()
    from report_tools.html_report import generate as generate_html_report  # type: ignore

    os.makedirs(output_dir, exist_ok=True)

    generate_html_report(
        results=results,
        output_dir=output_dir,
        baseline_dir=baseline_dir,
        report_path=report_filename,
        update_baseline=update_baseline,
        tracing_enabled=tracing_enabled,
        callgraph_path=callgraph_path,
        viewer_base_url=viewer_base_url,
    )

    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv))

