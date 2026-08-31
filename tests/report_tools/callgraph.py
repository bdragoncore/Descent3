#!/usr/bin/env python3
"""
report_tools/callgraph.py

Builds a unified caller→callee graph from trace JSON files.
Used by run_render_tests.py to produce build/callgraph.json for the HTML report.
Reuses build_dag() from generate_html so DAG construction is shared with per-trace HTML.
"""

from report_tools.generate_html import load_trace, build_dag


def build_call_graph(trace_paths: list) -> dict:
    """
    Build a unified caller→callee graph from a list of trace JSON paths.

    Returns a dict with "nodes" and "links" suitable for D3.js force layout.
    Links have "source", "target", "count", and "total_ns" (aggregate time).
    Uses build_dag() per trace and merges results.
    """
    nodes_map: dict[str, dict] = {}  # id -> node
    edges: dict[tuple[str, str], dict] = {}  # (src, tgt) -> { count, total_ns }

    for path in trace_paths:
        try:
            events = load_trace(path)
        except Exception:
            continue
        dag = build_dag(events)
        for n in dag["nodes"]:
            nid = n["id"]
            if nid not in nodes_map:
                nodes_map[nid] = {"id": nid, "label": n.get("label", nid)}
        for link in dag["links"]:
            src, tgt = link["source"], link["target"]
            key = (src, tgt)
            if key not in edges:
                edges[key] = {"count": 0, "total_ns": 0}
            edges[key]["count"] += link.get("count", 1)
            edges[key]["total_ns"] += link.get("total_ns", 0)

    return {
        "nodes": list(nodes_map.values()),
        "links": [
            {"source": src, "target": tgt, "count": e["count"], "total_ns": e["total_ns"]}
            for (src, tgt), e in edges.items()
        ],
    }
