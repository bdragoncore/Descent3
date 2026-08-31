#!/usr/bin/env python3
"""
Parse a callgrind.out file from valgrind --tool=callgrind --toggle-collect=FnName.
Filter noise, build a directed call graph via BFS from root, stop at endpoints,
write graph.json.
"""

import argparse
import json
import re
import sys
from collections import deque

NOISE_PREFIXES = (
    "std::",
    "void std::",
    "unsigned long",
    "unsigned int",
    "__gnu_cxx::",
    "__cxxabiv",
    "__cxa_",
    "testing::",
    "testing::internal::",
    "testing::TestInfo",
    "testing::Test::",
    "??",
    "<unknown>",
)

END_MARKERS = ("ReadPixels", "glReadPixels")


def _is_noise(name: str) -> bool:
    if not name or not name.strip():
        return True
    for p in NOISE_PREFIXES:
        if name.startswith(p):
            return True
    return False


def _is_endpoint(name: str) -> bool:
    for m in END_MARKERS:
        if m in name:
            return True
    return False


def _resolve_spec(s: str, table: dict, next_id: list) -> str:
    """Resolve fn= or fl= spec; update table; return resolved string or None for file."""
    s = s.strip()
    # spec=(N) or spec=(N) name
    m = re.match(r"^\((\d+)\)\s*(.*)$", s)
    if m:
        idx = int(m.group(1))
        rest = m.group(2).strip()
        if rest:
            table[idx] = rest
            return rest
        return table.get(idx, "")
    # spec=name
    if s:
        idx = next_id[0]
        next_id[0] += 1
        table[idx] = s
        return s
    return ""


def parse_callgrind(path: str) -> tuple[dict, dict, list]:
    """
    Parse callgrind file. Returns (fn_self_cost, fn_file, edges).
    fn_self_cost: fn -> self cost (int)
    fn_file: fn -> source file path
    edges: list of (from_fn, to_fn, calls, cost)
    """
    fn_table = {}
    file_table = {}
    fn_next = [1]
    fl_next = [1]

    fn_self_cost = {}
    fn_file = {}
    edges = []

    current_fn = None
    current_fl = None
    last_cfn = None
    in_header = True
    pending_calls = None  # (cfn, calls_count) then next line is inclusive cost

    with open(path, "r", encoding="utf-8", errors="replace") as f:
        for raw in f:
            line = raw.strip()
            if not line or line.startswith("#"):
                continue

            if in_header:
                if line.startswith("events:"):
                    in_header = False
                elif line.startswith("positions:"):
                    pass
                continue

            # Position specs
            if line.startswith("fl="):
                name = _resolve_spec(line[3:], file_table, fl_next)
                if name:
                    current_fl = name
            elif line.startswith("fn="):
                name = _resolve_spec(line[3:], fn_table, fn_next)
                if name:
                    current_fn = name
                    if current_fl and current_fn:
                        fn_file[current_fn] = current_fl
            elif line.startswith("cfl="):
                _resolve_spec(line[4:], file_table, fl_next)
            elif line.startswith("cfn="):
                cfn = _resolve_spec(line[4:], fn_table, fn_next)
                if cfn:
                    last_cfn = cfn
            elif line.startswith("calls="):
                rest = line[6:].strip()
                parts = rest.split()
                n = 1
                if parts:
                    try:
                        n = int(parts[0])
                    except ValueError:
                        pass
                pending_calls = (last_cfn, n)
                continue
            else:
                # Cost line: one or more subpositions (e.g. line number) then event counts
                parts = line.split()
                if not parts:
                    continue
                # Skip first token (subposition, e.g. line); next is first event count
                i = 1
                if i < len(parts):
                    try:
                        cost = int(parts[i])
                    except ValueError:
                        cost = 0
                else:
                    cost = 0

                if pending_calls is not None:
                    cfn, n = pending_calls
                    if cfn and current_fn:
                        edges.append((current_fn, cfn, n, cost))
                    pending_calls = None
                else:
                    if current_fn is not None:
                        fn_self_cost[current_fn] = fn_self_cost.get(current_fn, 0) + cost

    return fn_self_cost, fn_file, edges


def build_adjacency(edges: list) -> dict:
    """Build adjacency list: from_fn -> [(to_fn, calls, cost), ...]"""
    adj = {}
    for (f, t, c, cost) in edges:
        adj.setdefault(f, []).append((t, c, cost))
    return adj


def _resolve_root(root_hint: str, adj: dict, fn_self_cost: dict, edges: list) -> str:
    """
    Resolve root_hint (e.g. "D3EGLContextTest::BasicClear") to an actual node name
    (e.g. "D3EGLContextTest_BasicClear_Test::TestBody()") for BFS.
    Uses substring match so GTest-mangled names are found; fallback to root not called by others.
    """
    all_nodes = set(adj.keys()) | set(fn_self_cost.keys())
    if not all_nodes:
        return root_hint
    if root_hint and root_hint in all_nodes:
        return root_hint
    # Substring match: e.g. "D3EGLContextTest::BasicClear" -> "D3EGLContextTest_BasicClear" in fn
    match_key = root_hint.replace("::", "_") if root_hint else ""
    if match_key:
        matches = [fn for fn in all_nodes if match_key in fn and not _is_noise(fn)]
        if matches:
            return matches[0]
    # Fallback: node that is not called by any other (likely top-level)
    called_by_anyone = {e[1] for e in edges if not _is_noise(e[0])}
    candidates = [fn for fn in all_nodes if fn not in called_by_anyone and not _is_noise(fn)]
    if candidates:
        return min(candidates, key=lambda f: f.count("::"))
    return next(iter(all_nodes), root_hint)


def bfs_subgraph(root: str, adj: dict, fn_self_cost: dict, fn_file: dict) -> tuple[dict, list]:
    """
    BFS from root; skip noise; stop expanding at endpoints (but include endpoint node).
    Returns (nodes_dict, edges_list) for graph.json.
    """
    if not root or (root not in adj and root not in fn_self_cost):
        return {}, []

    nodes = {}
    out_edges = []
    seen = {root}
    q = deque([root])
    depth = {root: 0}

    while q:
        u = q.popleft()
        d = depth[u]
        if _is_noise(u):
            continue
        self_cost = fn_self_cost.get(u, 0)
        file_path = fn_file.get(u, "")
        short = u if len(u) <= 28 else u[:25] + "..."
        endpoint = _is_endpoint(u)
        nodes[u] = {
            "fn": u,
            "short_fn": short,
            "file": file_path,
            "self_cost": self_cost,
            "total_cost": 0,
            "calls_in": 0,
            "endpoint": endpoint,
        }
        if u not in adj:
            continue
        for (v, calls, cost) in adj[u]:
            if _is_noise(v):
                continue
            out_edges.append({"from": u, "to": v, "calls": calls, "cost": cost})
            nodes[u]["total_cost"] = nodes[u].get("total_cost", 0) + cost
            if v not in nodes:
                nodes[v] = {
                    "fn": v,
                    "short_fn": (v if len(v) <= 28 else v[:25] + "..."),
                    "file": fn_file.get(v, ""),
                    "self_cost": fn_self_cost.get(v, 0),
                    "total_cost": 0,
                    "calls_in": 0,
                    "endpoint": _is_endpoint(v),
                }
            nodes[v]["calls_in"] = nodes[v].get("calls_in", 0) + calls
            if not _is_endpoint(v) and v not in seen:
                seen.add(v)
                depth[v] = d + 1
                q.append(v)

    for e in out_edges:
        to_fn = e["to"]
        if to_fn in nodes:
            nodes[to_fn]["total_cost"] = nodes[to_fn].get("total_cost", 0) + nodes[to_fn].get("self_cost", 0)

    return nodes, out_edges


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Parse callgrind.out and write graph.json for call graph viewer",
    )
    parser.add_argument("callgrind_out", help="Path to callgrind.out file")
    parser.add_argument("graph_json", help="Output path for graph.json")
    parser.add_argument("root_fn", help="Root function name (e.g. Suite::TestName)")
    args = parser.parse_args()

    fn_self_cost, fn_file, edges = parse_callgrind(args.callgrind_out)
    adj = build_adjacency(edges)
    bfs_root = _resolve_root(args.root_fn, adj, fn_self_cost, edges)
    nodes, out_edges = bfs_subgraph(bfs_root, adj, fn_self_cost, fn_file)

    for k in nodes:
        nodes[k]["total_cost"] = nodes[k].get("total_cost", 0) or nodes[k].get("self_cost", 0)

    # Output root as the user-friendly hint (e.g. D3EGLContextTest::BasicClear)
    out = {
        "root": args.root_fn,
        "nodes": nodes,
        "edges": out_edges,
    }
    with open(args.graph_json, "w") as f:
        json.dump(out, f, indent=2)
    return 0


if __name__ == "__main__":
    sys.exit(main())
