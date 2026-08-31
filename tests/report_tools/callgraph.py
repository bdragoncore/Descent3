#!/usr/bin/env python3
"""
report_tools/callgraph.py

Builds a unified caller→callee graph from trace JSON files.
Used by run_render_tests.py to produce build/callgraph.json for the HTML report.
"""

from report_tools.generate_html import load_trace

ROOT_ID = "<root>"


def build_call_graph(trace_paths: list) -> dict:
    """
    Build a unified caller→callee graph from a list of trace JSON paths.

    Returns a dict with "nodes" and "links" suitable for D3.js force layout.
    Reuses load_trace() from generate_html so the event format is shared.
    Root-level calls (stack empty on enter) are recorded as <root> → fn.
    """
    nodes: dict[str, dict] = {}  # fn_name -> node dict
    links: dict[tuple[str, str], int] = {}  # (caller, callee) -> call count

    for path in trace_paths:
        try:
            events = load_trace(path)
        except Exception:
            continue
        stack: list[str] = []
        for event in events:
            fn = event.get("fn", "??")
            if event.get("type") == "enter":
                if fn not in nodes:
                    nodes[fn] = {"id": fn, "label": fn}
                caller = stack[-1] if stack else ROOT_ID
                if caller not in nodes:
                    nodes[caller] = {"id": caller, "label": caller}
                edge = (caller, fn)
                links[edge] = links.get(edge, 0) + 1
                stack.append(fn)
            elif event.get("type") == "exit":
                if stack:
                    stack.pop()

    return {
        "nodes": list(nodes.values()),
        "links": [
            {"source": src, "target": tgt, "count": cnt}
            for (src, tgt), cnt in links.items()
        ],
    }
