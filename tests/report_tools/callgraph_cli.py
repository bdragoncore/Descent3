#!/usr/bin/env python3
"""
Small CLI wrapper around report_tools.callgraph.build_call_graph.

Usage:
    python3 tests/report_tools/callgraph_cli.py OUT_PATH [TRACE_JSON ...]

Writes a callgraph JSON file compatible with html_report.py.
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
    if len(argv) < 2:
        print(
            "Usage: callgraph_cli.py OUT_PATH [TRACE_JSON ...]",
            file=sys.stderr,
        )
        return 1

    out_path = argv[1]
    trace_paths = argv[2:]

    _add_tests_to_sys_path()
    from report_tools.callgraph import build_call_graph  # type: ignore

    if trace_paths:
        graph = build_call_graph(trace_paths)
    else:
        graph = {}

    os.makedirs(os.path.dirname(out_path) or ".", exist_ok=True)
    with open(out_path, "w") as f:
        json.dump(graph, f, indent=2)

    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv))

