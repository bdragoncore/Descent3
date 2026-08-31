#!/usr/bin/env python3
"""
report_tools/generate_html.py

Converts a per-test trace JSON file (produced by tracer.cpp) into a
self-contained HTML file with an interactive D3.js collapsible call tree.

Can be used as a standalone script or imported as a module:

    from report_tools.generate_html import generate_html
    html_str = generate_html("trace__Suite__Test.json")
"""

import json
import os
import sys
import html as html_mod


def load_trace(path: str) -> list:
    with open(path) as f:
        return json.load(f)


# Max nodes to embed in HTML so the call graph can render (browser limit)
MAX_TREE_NODES = 4000

# End of path: preferred = ReadPixels; fallback when symbols missing (??) = final GL sync/swap
END_FN_PATTERNS = ["ReadPixels"]
FINAL_GL_FALLBACK_PATTERNS = [
    "glFinish",
    "glFlush",
    "eglSwapBuffers",
    "SwapBuffers",
    "SDL_GL_SwapWindow",
    "glXSwapBuffers",
]


def _get_test_method_name(trace_path: str) -> str:
    """From trace__Suite__TestName.json return TestName (e.g. BasicClear)."""
    base = os.path.basename(trace_path).replace(".json", "").replace("trace__", "")
    if "__" in base:
        return base.split("__")[-1]
    return base


def _fn_matches_end(fn: str) -> bool:
    """True if fn is the path end (e.g. ReadPixels)."""
    if not fn:
        return False
    for pat in END_FN_PATTERNS:
        if pat in fn:
            return True
    return False


def _fn_matches_final_gl_fallback(fn: str) -> bool:
    """True if fn looks like a final GL sync/swap (fallback when ReadPixels is ??)."""
    if not fn:
        return False
    for pat in FINAL_GL_FALLBACK_PATTERNS:
        if pat in fn:
            return True
    return False


def find_path_from_test_to_readpixels(events: list, trace_path: str) -> list:
    """
    Return the path from the test function (e.g. BasicClear) to ReadPixels.
    Start: first frame whose fn contains the test method name from the trace filename.
    End: last frame whose fn contains ReadPixels.
    If the test name is never seen, start at the first frame (first child of root).
    Path is a list of node dicts with fn, ts, duration, addr.
    """
    test_method = _get_test_method_name(trace_path)
    root = {"fn": "root", "ts": 0, "duration": 0, "addr": ""}
    stack = [root]
    start_depth = None  # stack index of the test function frame when we're inside it
    last_path = None

    for event in events:
        if event["type"] == "enter":
            node = {
                "fn": event.get("fn", "??"),
                "ts": event.get("ts_ns", 0),
                "duration": 0,
                "addr": event.get("addr", ""),
            }
            stack.append(node)

            if start_depth is None and test_method and test_method in node["fn"]:
                start_depth = len(stack) - 1

            if start_depth is None and len(stack) == 2:
                start_depth = 1

            if start_depth is not None:
                is_end = _fn_matches_end(node["fn"])
                is_fallback = _fn_matches_final_gl_fallback(node["fn"])
                if is_end or is_fallback:
                    last_path = list(stack[start_depth:])

        elif event["type"] == "exit" and len(stack) > 1:
            node = stack.pop()
            node["duration"] = event.get("ts_ns", 0) - node["ts"]

    return last_path if last_path else []


def path_to_slim_tree(path: list) -> dict:
    """Build a single-chain tree: root -> path[0] -> path[1] -> ... -> path[-1]."""
    root = {"fn": "root", "ts": 0, "duration": 0, "children": []}
    if not path:
        return root
    for node in path:
        node.setdefault("children", [])
    for i in range(len(path) - 1):
        path[i]["children"] = [path[i + 1]]
    path[-1]["children"] = []
    # Mark endpoint on last node for styling in the viewer
    if path:
        path[-1]["endpoint"] = True
        root["children"] = [path[0]]
        root["ts"] = path[0]["ts"]
        root["duration"] = path[-1]["ts"] + path[-1]["duration"] - path[0]["ts"]
    return root


def merge_repeated_calls(node: dict) -> None:
    """
    Merge repeated sibling calls with the same function name into a single node.
    Aggregates duration and call counts so the viewer can display calls/avg time.
    """
    children = node.get("children") or []
    if not children:
        return

    from collections import defaultdict

    groups: dict[str, list[dict]] = defaultdict(list)
    for child in children:
        groups[child.get("fn", "??")].append(child)

    merged: list[dict] = []
    for fn, instances in groups.items():
        if len(instances) == 1:
            merged.append(instances[0])
            continue

        base = instances[0]
        calls = len(instances)
        total_duration = sum(c.get("duration", 0) for c in instances)
        base["calls"] = base.get("calls", 1) + (calls - 1)
        base["duration"] = total_duration
        merged.append(base)

    node["children"] = merged
    for child in node["children"]:
        merge_repeated_calls(child)


def annotate_stats(node: dict) -> None:
    """
    Add per-node statistics used by the visualization:
    - child_count
    - calls
    - total_time_ns
    - avg_time_ns
    """
    children = node.get("children") or []
    node["child_count"] = len(children)
    calls = node.get("calls", 1)
    node["calls"] = calls
    total_ns = node.get("duration", 0)
    node["total_time_ns"] = total_ns
    node["avg_time_ns"] = total_ns // calls if calls else 0

    for child in children:
        annotate_stats(child)


def build_tree(events: list) -> dict:
    """Convert flat enter/exit event list into a nested call tree."""
    root = {"fn": "root", "ts": 0, "duration": 0, "children": []}
    stack = [root]

    for event in events:
        if event["type"] == "enter":
            node = {
                "fn": event["fn"],
                "addr": event.get("addr", ""),
                "ts": event["ts_ns"],
                "duration": 0,
                "children": [],
            }
            stack[-1]["children"].append(node)
            stack.append(node)
        elif event["type"] == "exit":
            if len(stack) > 1:
                node = stack.pop()
                node["duration"] = event["ts_ns"] - node["ts"]

    if root["children"]:
        first_ts = root["children"][0]["ts"]
        last = root["children"][-1]
        root["ts"] = first_ts
        root["duration"] = last["ts"] + last["duration"] - first_ts

    return root


def _count_nodes(node: dict) -> int:
    n = 1
    for c in node.get("children") or []:
        n += _count_nodes(c)
    return n


def _truncate_tree(node: dict, cap: int, seen: list) -> None:
    """In-place: keep at most cap nodes (depth-first), replace rest with [truncated] stub."""
    if seen[0] >= cap:
        return
    seen[0] += 1
    children = node.get("children") or []
    if not children:
        return
    kept = []
    for i, child in enumerate(children):
        if seen[0] >= cap:
            break
        kept.append(child)
        _truncate_tree(child, cap, seen)
    if len(kept) < len(children):
        dropped_count = sum(_count_nodes(c) for c in children[len(kept):])
        kept.append({
            "fn": f"[+{dropped_count} nodes truncated — trace too large to show in full]",
            "addr": "",
            "ts": 0,
            "duration": 0,
            "children": [],
        })
    node["children"] = kept


def generate_html(trace_path: str) -> str:
    """Return a self-contained HTML string for the given trace JSON file."""
    events = load_trace(trace_path)
    total_events = len(events)

    path = find_path_from_test_to_readpixels(events, trace_path)
    test_method = _get_test_method_name(trace_path)
    if path:
        tree = path_to_slim_tree(path)
        end_label = "ReadPixels" if path and "ReadPixels" in path[-1].get("fn", "") else "final GL"
        truncated_note = f" &nbsp;|&nbsp; Path: {test_method} → {end_label} ({len(path)} nodes)"
    else:
        tree = build_tree(events)
        node_count = _count_nodes(tree)
        truncated_note = ""
        if node_count > MAX_TREE_NODES:
            _truncate_tree(tree, MAX_TREE_NODES, [0])
            truncated_note = f" &nbsp;|&nbsp; Tree truncated to {MAX_TREE_NODES} nodes (full trace has {node_count})"

    # Merge repeated sibling calls and annotate stats for visualization
    merge_repeated_calls(tree)
    annotate_stats(tree)

    tree_json = json.dumps(tree, indent=2)
    # Prevent </script> in JSON from closing the script tag in HTML
    tree_json_safe = tree_json.replace("</script>", "<\\/script>")
    test_name = os.path.basename(trace_path).replace(".json", "").replace("trace__", "")
    safe_name = html_mod.escape(test_name)

    return f"""<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Render Trace: {safe_name}</title>
  <script src="https://cdnjs.cloudflare.com/ajax/libs/d3/7.8.5/d3.min.js"></script>
  <style>
    * {{ box-sizing: border-box; margin: 0; padding: 0; }}
    body {{ font-family: 'Menlo','Consolas',monospace; background:#0d1117; color:#c9d1d9; padding:20px; }}
    h1 {{ font-size:18px; margin-bottom:4px; color:#58a6ff; }}
    .subtitle {{ font-size:12px; color:#8b949e; margin-bottom:20px; }}
    #controls {{ margin-bottom:12px; }}
    button {{ background:#21262d; color:#c9d1d9; border:1px solid #30363d; padding:6px 14px;
              border-radius:6px; cursor:pointer; font-size:12px; margin-right:8px; }}
    button:hover {{ background:#30363d; }}
    #tree-container {{ background:#161b22; border:1px solid #30363d; border-radius:8px;
                       overflow:auto; padding:20px; min-height:400px; }}
    .link {{ stroke:#30363d; stroke-width:1.5px; }}
    .tooltip {{ position:fixed; background:#1c2128; border:1px solid #30363d; border-radius:6px;
                padding:10px 14px; font-size:11px; pointer-events:none; opacity:0;
                transition:opacity 0.15s; max-width:500px; word-break:break-all;
                z-index:100; color:#c9d1d9; }}
    .tooltip .fn  {{ color:#79c0ff; font-weight:bold; margin-bottom:4px; }}
    .tooltip .dur {{ color:#56d364; }}
    .tooltip .addr {{ color:#8b949e; }}
  </style>
</head>
<body>
  <h1>&#9654; Render Trace: {safe_name}</h1>
  <div class="subtitle">{total_events} events{truncated_note} &nbsp;|&nbsp; Click nodes to expand/collapse</div>
  <div id="controls">
    <button onclick="expandAll()">Expand All</button>
    <button onclick="collapseAll()">Collapse All</button>
    <button onclick="resetZoom()">Reset Zoom</button>
  </div>
  <div id="tree-container"><svg id="tree-svg"></svg></div>
  <div class="tooltip" id="tooltip"></div>
<script>
const RAW_TREE = {tree_json_safe};
const COLORS = ['#58a6ff','#79c0ff','#56d364','#3fb950','#f0883e','#d29922','#bc8cff','#ff7b72'];
const colorFor = depth => COLORS[depth % COLORS.length];
const fmtDur = ns => ns < 1000 ? ns+'ns' : ns < 1e6 ? (ns/1000).toFixed(2)+'us'
                   : ns < 1e9  ? (ns/1e6).toFixed(2)+'ms' : (ns/1e9).toFixed(3)+'s';
const margin = {{top:20,right:300,bottom:20,left:80}};
const nodeH  = 28;
const NODE_W = 200;
const NODE_H = 80;
let svg, g, root, zoom, nodeId = 0;

function init() {{
  const w = Math.max(document.getElementById('tree-container').clientWidth - 40, 900);
  svg  = d3.select('#tree-svg').attr('width', w).attr('height', 600);
  zoom = d3.zoom().scaleExtent([0.2,4]).on('zoom', e => g.attr('transform', e.transform));
  svg.call(zoom);
  svg.append('defs').append('marker')
    .attr('id', 'arrowhead')
    .attr('viewBox', '0 -5 10 10')
    .attr('refX', 10)
    .attr('refY', 0)
    .attr('markerWidth', 6)
    .attr('markerHeight', 6)
    .attr('orient', 'auto')
    .append('path')
    .attr('d', 'M0,-5L10,0L0,5')
    .attr('fill', '#58a6ff');
  g    = svg.append('g').attr('transform',`translate(${{margin.left}},${{margin.top}})`);
  root = d3.hierarchy(RAW_TREE);
  root.x0 = root.y0 = 0;
  root.children && root.children.forEach(collapse);
  update(root);
}}

const collapse = d => {{ if (d.children) {{ d._children = d.children; d._children.forEach(collapse); d.children = null; }} }};
const expand   = d => {{ if (d._children) {{ d.children = d._children; d._children = null; d.children.forEach(expand); }} }};
const expandAll   = () => {{ root.children && root.children.forEach(expand);   update(root); }};
const collapseAll = () => {{ root.children && root.children.forEach(collapse); update(root); }};
const resetZoom   = () => svg.transition().duration(400)
  .call(zoom.transform, d3.zoomIdentity.translate(margin.left, margin.top));

function updateLinks() {{
  const links = root.links();
  const lk = g.selectAll('line.link').data(links, d => d.target.id);
  const lkE = lk.enter().insert('line', 'g')
    .attr('class','link')
    .attr('marker-end', 'url(#arrowhead)')
    .attr('x1', d => d.source.y0 + NODE_W / 2)
    .attr('y1', d => d.source.x0)
    .attr('x2', d => d.source.y0 + NODE_W / 2)
    .attr('y2', d => d.source.x0);
  lkE.merge(lk).transition().duration(250)
    .attr('x1', d => d.source.y + NODE_W / 2)
    .attr('y1', d => d.source.x)
    .attr('x2', d => d.target.y - NODE_W / 2)
    .attr('y2', d => d.target.x);
  lk.exit().transition().duration(250)
    .attr('x1', d => d.source.y + NODE_W / 2)
    .attr('y1', d => d.source.x)
    .attr('x2', d => d.source.y + NODE_W / 2)
    .attr('y2', d => d.source.x)
    .remove();
}}

function update(src) {{
  d3.tree().nodeSize([nodeH, 300])(root);
  let nc = 0; root.each(() => nc++);
  svg.attr('height', Math.max(600, nc * nodeH + margin.top + margin.bottom));

  const nodes = root.descendants(), links = root.links();
  const nd = g.selectAll('g.node').data(nodes, d => d.id || (d.id = ++nodeId));
  const ndE = nd.enter().append('g').attr('class','node')
    .attr('transform', `translate(${{src.y0}},${{src.x0}})`)
    .on('click', (_,d) => {{ if(d.children){{d._children=d.children;d.children=null;}}
                              else{{d.children=d._children;d._children=null;}} update(d); }})
    .on('mouseover', showTooltip).on('mousemove', moveTooltip).on('mouseout', hideTooltip);

  const card = ndE.append('foreignObject')
    .attr('width', NODE_W)
    .attr('height', NODE_H)
    .attr('x', -NODE_W / 2)
    .attr('y', -NODE_H / 2)
    .append('xhtml:div')
    .style('width', NODE_W + 'px')
    .style('height', NODE_H + 'px')
    .style('background', d => d.data.endpoint ? '#2d1a0e' : '#161b22')
    .style('border', d => d.data.endpoint ? '2px solid #f0883e' : '2px solid ' + colorFor(d.depth || 0))
    .style('border-radius', '6px')
    .style('padding', '6px 10px')
    .style('font-family', 'Menlo, Consolas, monospace')
    .style('font-size', '11px')
    .style('color', '#c9d1d9')
    .style('cursor', 'pointer')
    .html(d => {{
      const fn = d.data.fn || '??';
      const title = fn.length > 24 ? fn.slice(0,22) + '...' : fn;
      const calls = d.data.calls || 1;
      const total = d.data.total_time_ns != null ? d.data.total_time_ns : (d.data.duration || 0);
      const avg = d.data.avg_time_ns != null ? d.data.avg_time_ns : (d.data.duration || 0);
      const hiddenChildren = (!d.children && d._children) ? (d.data.child_count || d._children.length || 0) : 0;
      const endpointLabel = d.data.endpoint ? '<div style=\"color:#f0883e;font-weight:bold;margin-top:2px;\">ENDPOINT</div>' : '';
      const hiddenLabel = hiddenChildren ? `<div style=\"color:#8b949e;\">hidden: <span style=\"color:#c9d1d9;\">${{hiddenChildren}} children</span></div>` : '';
      const badge = hiddenChildren ? '[+]' : '';
      return `
        <div style=\"color:${{d.data.endpoint ? '#f0883e' : '#79c0ff'}};font-weight:bold;white-space:nowrap;overflow:hidden;text-overflow:ellipsis;\">
          ${'{'}title{'}'} <span style=\"float:right;\">${'{'}badge{'}'}</span>
        </div>
        <div style=\"border-top:1px solid #30363d;margin:4px 0;\"></div>
        <div style=\"color:#8b949e;\">calls: <span style=\"color:#c9d1d9;\">${'{'}calls{'}'}</span></div>
        <div style=\"color:#8b949e;\">time: <span style=\"color:#56d364;\">${'{'}fmtDur(total){'}'}</span></div>
        <div style=\"color:#8b949e;\">avg: <span style=\"color:#56d364;\">${'{'}fmtDur(avg){'}'}</span></div>
        ${'{'}hiddenLabel{'}'}
        ${'{'}endpointLabel{'}'}
      `;
    }});

  const drag = d3.drag()
    .on('start', (event, d) => {{ d.dragging = true; }})
    .on('drag', (event, d) => {{
      d.x = event.y;
      d.y = event.x;
      d3.select(event.sourceEvent.target.closest('g.node'))
        .attr('transform', `translate(${{d.y}},${{d.x}})`);
      updateLinks();
    }})
    .on('end', (event, d) => {{ d.dragging = false; }});

  ndE.call(drag);

  const ndU = ndE.merge(nd);
  ndU.transition().duration(250).attr('transform', d => `translate(${{d.y}},${{d.x}})`);
  nd.exit().transition().duration(250).attr('transform', `translate(${{src.y}},${{src.x}})`).remove();
  nodes.forEach(d => {{ d.x0=d.x; d.y0=d.y; }});
  updateLinks();
}}

const tip = document.getElementById('tooltip');
function showTooltip(ev, d) {{
  tip.innerHTML = `<div class="fn">${{d.data.fn||'??'}}</div>
    <div class="dur">Duration: ${{fmtDur(d.data.duration)}}</div>
    <div class="addr">Addr: ${{d.data.addr||'n/a'}}</div>
    <div>Children: ${{(d.children||d._children||[]).length}}</div>`;
  tip.style.opacity = '1'; moveTooltip(ev);
}}
function moveTooltip(ev) {{ tip.style.left=(ev.clientX+16)+'px'; tip.style.top=(ev.clientY+16)+'px'; }}
function hideTooltip()   {{ tip.style.opacity='0'; }}
init();
</script>
</body>
</html>"""


if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python3 generate_html.py <trace_file.json> [...]")
        sys.exit(1)
    for trace_path in sys.argv[1:]:
        if not os.path.exists(trace_path):
            print(f"[!] Not found: {trace_path}")
            continue
        out_path = trace_path.replace(".json", ".html")
        with open(out_path, "w") as f:
            f.write(generate_html(trace_path))
        print(f"[OK] {trace_path} -> {out_path}")
