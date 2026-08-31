from dataclasses import dataclass, field
from datetime import datetime
from typing import Optional, Tuple
import json
import os
import shutil


@dataclass
class TestReportData:
    test_name: str
    test_suite: str
    executable: str
    duration_ms: int
    md5_hash: Optional[str]
    previous_md5: Optional[str]
    status: str  # PASSED/FAILED
    output_text: str
    environment: dict
    render_functions: list
    png_filename: str
    trace_html_filename: Optional[str] = None
    callgraphs: list = field(default_factory=list)


@dataclass
class TestResult:
    exe: str
    passed: bool
    pngs: list
    md5s: list
    traces: list


class HTMLReportGenerator:
    def __init__(
        self,
        output_dir: str,
        baseline_dir: str = None,
        update_baseline: bool = False,
        callgraph_path: Optional[str] = None,
        viewer_base_url: Optional[str] = None,
    ):
        self.output_dir = output_dir
        self.baseline_dir = baseline_dir
        self.update_baseline = update_baseline
        self.callgraph_path = callgraph_path
        self.viewer_base_url = viewer_base_url
        self.test_results: list[TestReportData] = []
        self.generated_at = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        self.tracing_enabled = True

    def set_tracing_enabled(self, enabled: bool):
        self.tracing_enabled = enabled

    def add_test_result(self, result: TestReportData):
        self.test_results.append(result)

    def generate(self, report_filename: str = "render_report.html") -> str:
        # Handle baseline update
        if self.update_baseline and self.baseline_dir:
            os.makedirs(self.baseline_dir, exist_ok=True)
            for result in self.test_results:
                if result.png_filename:
                    src = os.path.join(self.output_dir, result.png_filename)
                    dst = os.path.join(self.baseline_dir, result.png_filename)
                    if os.path.exists(src):
                        shutil.copy2(src, dst)
                if result.md5_hash:
                    md5_filename = (
                        result.png_filename.replace(".png", ".md5")
                        if result.png_filename
                        else None
                    )
                    if md5_filename:
                        src = os.path.join(self.output_dir, md5_filename)
                        dst = os.path.join(self.baseline_dir, md5_filename)
                        if os.path.exists(src):
                            shutil.copy2(src, dst)

        report_path = os.path.join(self.output_dir, report_filename)
        os.makedirs(self.output_dir, exist_ok=True)

        html = self._build_html()

        with open(report_path, "w") as f:
            f.write(html)

        return report_path

    def _load_baseline_md5(self, test_name: str) -> Optional[str]:
        """Load baseline MD5 for a test if available."""
        if not self.baseline_dir:
            return None

        md5_file = os.path.join(self.baseline_dir, f"Test{test_name}.md5")
        if os.path.exists(md5_file):
            with open(md5_file, "r") as f:
                return f.read().strip()
        return None

    def _load_callgraph_for_report(
        self, top_n: int = 200
    ) -> Tuple[Optional[str], Optional[str]]:
        """
        Load call graph JSON from callgraph_path, limit to top_n nodes by total call count.
        Returns (json_str_for_embed, truncation_note_str_or_None).
        """
        if not self.callgraph_path or not os.path.exists(self.callgraph_path):
            return None, None
        try:
            with open(self.callgraph_path) as f:
                data = json.load(f)
        except Exception:
            return None, None
        nodes = data.get("nodes", [])
        links = data.get("links", [])
        if not nodes:
            return None, None

        # Total call count per node id (as source or target)
        count_by_id = {}
        for link in links:
            src, tgt = link.get("source"), link.get("target")
            cnt = link.get("count", 1)
            count_by_id[src] = count_by_id.get(src, 0) + cnt
            count_by_id[tgt] = count_by_id.get(tgt, 0) + cnt

        # Top N node ids
        sorted_ids = sorted(
            count_by_id.keys(),
            key=lambda i: count_by_id.get(i, 0),
            reverse=True,
        )[:top_n]
        keep = set(sorted_ids)

        nodes_top = [n for n in nodes if n.get("id") in keep]
        links_top = [
            L for L in links if L.get("source") in keep and L.get("target") in keep
        ]

        out = {"nodes": nodes_top, "links": links_top}
        note = None
        if len(nodes) > top_n:
            note = f"Showing top {top_n} of {len(nodes)} nodes."
        return json.dumps(out), note

    def _build_html(self) -> str:
        total = len(self.test_results)
        passed = sum(1 for r in self.test_results if r.status == "PASSED")
        failed = total - passed

        # Count MD5 regressions
        md5_regressions = 0
        for r in self.test_results:
            if r.md5_hash and r.previous_md5 and r.md5_hash != r.previous_md5:
                md5_regressions += 1

        results_html = ""
        for result in self.test_results:
            results_html += self._build_test_result(result)

        # Determine baseline update message
        baseline_msg = ""
        if self.update_baseline:
            baseline_msg = '<div class="baseline-msg">Baseline updated with current MD5 hashes</div>'

        # Call graph section: always show button; panel shows graph when data exists, else message
        cg_json, cg_note = self._load_callgraph_for_report()
        has_graph = cg_json is not None
        if has_graph:
            cg_json_safe = cg_json.replace("</script>", "<\\/script>")
            note_esc = (cg_note or "").replace("<", "&lt;").replace(">", "&gt;")
            panel_inner = f"""
                <h2 style="margin-top:0;">Call graph (this run)</h2>
                <p class="callgraph-note timestamp" id="callgraph-note">{note_esc}</p>
                <div id="callgraph-container" style="background:#161b22;border:1px solid #30363d;border-radius:8px;min-height:400px;padding:20px;"></div>"""
            d3_script = f"""
        <script src="https://cdnjs.cloudflare.com/ajax/libs/d3/7.8.5/d3.min.js"></script>
        <script>
        (function() {{
            const RAW = {cg_json_safe};
            const NOTE = {json.dumps(cg_note)};
            if (NOTE) document.getElementById("callgraph-note").textContent = NOTE;
            const width = document.getElementById("callgraph-container").clientWidth || 900;
            const height = 500;
            const svg = d3.select("#callgraph-container").append("svg").attr("width", width).attr("height", height);
            const g = svg.append("g");
            const color = d3.scaleOrdinal(["#58a6ff","#79c0ff","#56d364","#3fb950","#f0883e","#d29922","#bc8cff","#ff7b72"]);
            const nodes = RAW.nodes.map(d => ({{ ...d }}));
            const links = RAW.links.map(d => ({{ ...d }}));
            const simulation = d3.forceSimulation(nodes)
                .force("link", d3.forceLink(links).id(x => x.id).distance(80))
                .force("charge", d3.forceManyBody().strength(-200))
                .force("center", d3.forceCenter(width/2, height/2));
            const maxTotalNs = RAW.links.length ? Math.max(...RAW.links.map(l => l.total_ns || 0), 1) : 1;
            const linkGrp = g.append("g");
            const linkSel = linkGrp.selectAll("line").data(links).join("line")
                .attr("stroke", "#30363d")
                .attr("stroke-width", d => {{
                    const tn = d.total_ns;
                    if (tn == null || tn === 0) return 1;
                    return Math.max(1, 1 + 2 * (tn / maxTotalNs));
                }});
            linkSel.append("title").text(d => {{
                const cnt = d.count != null ? d.count : "";
                const tn = d.total_ns != null ? d.total_ns : "";
                const fmt = n => n < 1e6 ? n + "ns" : (n/1e6).toFixed(2) + "ms";
                if (cnt !== "" && tn !== "") return "calls: " + cnt + ", time: " + fmt(tn);
                if (cnt !== "") return "calls: " + cnt;
                return "";
            }});
            const node = g.append("g").selectAll("g").data(nodes).join("g")
                .attr("cursor", "pointer")
                .call(d3.drag().on("start", (e,d) => {{ e.sourceEvent.stopPropagation(); if (!e.active) simulation.alphaTarget(0.3).restart(); d.fx = d.x; d.fy = d.y; }})
                    .on("drag", (e,d) => {{ d.fx = e.x; d.fy = e.y; }})
                    .on("end", (e,d) => {{ if (!e.active) simulation.alphaTarget(0); d.fx = null; d.fy = null; }}));
            node.append("circle").attr("r", 6).attr("fill", d => color(d.id)).attr("stroke", "#fff").attr("stroke-width", 1.5);
            node.append("text").attr("x", 10).attr("dy", "0.35em").text(d => (d.label || d.id).length > 40 ? (d.label || d.id).slice(0,38)+"…" : (d.label || d.id)).attr("fill", "#c9d1d9").attr("font-size", "10px");
            node.append("title").text(d => d.label || d.id);
            simulation.on("tick", () => {{
                linkSel.attr("x1", d => d.source.x).attr("y1", d => d.source.y).attr("x2", d => d.target.x).attr("y2", d => d.target.y);
                node.attr("transform", d => `translate(${{d.x}},${{d.y}})`);
            }});
        }})();
        </script>"""
        else:
            panel_inner = """
                <p class="timestamp">Call graph is generated when tracing is enabled. Run the tests with tracing (e.g. <code>python tests/render/run_render_tests.py --build-dir build --tracing</code> or ensure the build has instrumentation and tracer library) to see the graph.</p>"""
            d3_script = ""
        callgraph_section = f"""
        <div class="summary" style="margin-bottom:20px;">
            <button type="button" id="callgraph-toggle" class="trace-link" style="cursor:pointer;border:none;margin-bottom:10px;">Show call graph</button>
            <div id="callgraph-section" style="display:none;">
                {panel_inner}
            </div>
        </div>
        <script>
        (function() {{
            const btn = document.getElementById("callgraph-toggle");
            const section = document.getElementById("callgraph-section");
            if (!btn || !section) return;
            btn.onclick = function() {{
                const visible = section.style.display !== "none";
                section.style.display = visible ? "none" : "block";
                btn.textContent = visible ? "Show call graph" : "Hide call graph";
            }};
        }})();
        </script>
        {d3_script}
"""

        return f"""<!DOCTYPE html>
<html>
<head>
    <meta charset="utf-8">
    <title>Render Test Report</title>
    <style>
        :root {{
            --bg-primary:     #0f1117;
            --bg-secondary:   #161b22;
            --bg-tertiary:    #21262d;
            --text-primary:   #f1f5f9;
            --text-secondary: #cbd5e1;
            --text-muted:     #94a3b8;
            --accent:         #8b5cf6;
            --accent-hover:   #a78bfa;
            --accent-primary: #58a6ff;
            --accent-hover-blue: #79c0ff;
            --border:         #2d3748;
            --success:        #56d364;
            --success-bg:     rgba(35, 134, 54, 0.2);
            --error:          #ff7b72;
            --error-bg:       rgba(218, 54, 51, 0.2);
            --warning:        #d29922;
            --warning-bg:     rgba(158, 106, 3, 0.2);
            --shadow:         rgba(0,0,0,0.4);
        }}
        
        body {{ 
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, Oxygen, Ubuntu, sans-serif;
            margin: 0; 
            padding: 20px; 
            background: var(--bg-primary);
            color: var(--text-primary);
            line-height: 1.6;
        }}
        
        .container {{ 
            max-width: 1200px; 
            margin: 0 auto;
        }}
        
        h1 {{ 
            color: var(--text-primary); 
            border-bottom: 2px solid var(--accent); 
            padding-bottom: 15px;
            font-size: 2.5em;
            font-weight: 700;
            margin-bottom: 30px;
        }}
        
        .summary {{ 
            background: var(--bg-secondary); 
            padding: 30px; 
            border-radius: 16px; 
            margin-bottom: 30px;
            box-shadow: 0 10px 40px var(--shadow);
            border: 1px solid var(--border);
        }}
        
        .summary-stats {{ 
            display: flex; 
            gap: 40px;
            margin-top: 20px;
        }}
        
        .stat {{ 
            text-align: center;
            padding: 20px;
            background: var(--bg-tertiary);
            border-radius: 12px;
            border: 1px solid var(--border);
            flex: 1;
        }}
        
        .stat-value {{ 
            font-size: 2.5em; 
            font-weight: 700;
            color: var(--text-primary);
        }}
        
        .stat-label {{ 
            color: var(--text-muted);
            font-size: 0.9em;
            text-transform: uppercase;
            letter-spacing: 1px;
            margin-top: 8px;
        }}
        
        .passed {{ color: var(--success); }}
        .failed {{ color: var(--error); }}
        .regression {{ color: var(--warning); }}
        
        .header-info {{ 
            margin-bottom: 20px;
            color: var(--text-secondary);
        }}
        
        .header-info span {{ 
            margin-right: 25px;
            display: inline-block;
        }}
        
        .test-result {{ 
            background: var(--bg-secondary); 
            margin-bottom: 25px; 
            border-radius: 16px;
            box-shadow: 0 10px 40px var(--shadow); 
            overflow: hidden;
            border: 1px solid var(--border);
        }}
        
        .test-header {{ 
            background: var(--bg-tertiary); 
            padding: 20px 25px; 
            border-bottom: 1px solid var(--border);
            display: flex; 
            justify-content: space-between; 
            align-items: center; 
        }}
        
        .test-name {{ 
            font-weight: 600; 
            font-size: 1.2em;
            color: var(--text-primary);
        }}
        
        .test-status {{ 
            padding: 8px 16px; 
            border-radius: 8px; 
            font-weight: 600;
            font-size: 0.9em;
            text-transform: uppercase;
            letter-spacing: 1px;
        }}
        
        .test-status.passed {{ 
            background: var(--success-bg); 
            color: var(--success);
            border: 1px solid var(--success);
        }}
        
        .test-status.failed {{ 
            background: var(--error-bg); 
            color: var(--error);
            border: 1px solid var(--error);
        }}
        
        .test-details {{ 
            padding: 25px; 
        }}
        
        .detail-table {{ 
            width: 100%; 
            border-collapse: collapse;
            margin-bottom: 20px;
        }}
        
        .detail-table th, .detail-table td {{ 
            padding: 12px 15px; 
            text-align: left; 
            border-bottom: 1px solid var(--border);
        }}
        
        .detail-table th {{ 
            color: var(--text-muted); 
            font-weight: 500; 
            width: 150px; 
            text-transform: uppercase;
            font-size: 0.85em;
            letter-spacing: 1px;
        }}
        
        .detail-table td {{
            color: var(--text-secondary);
        }}
        
        .md5-match {{ 
            color: var(--success); 
            font-weight: 500;
        }}
        
        .md5-mismatch {{ 
            background: var(--warning-bg); 
            padding: 6px 12px; 
            border-radius: 6px;
            color: var(--warning);
            border: 1px solid var(--warning);
        }}
        
        .md5-no-baseline {{ 
            color: var(--warning);
            font-weight: 500;
        }}
        
        .trace-link {{ 
            display: inline-block; 
            margin-top: 15px; 
            padding: 10px 20px; 
            background: var(--accent); 
            color: white; 
            text-decoration: none; 
            border-radius: 8px; 
            font-size: 0.95em;
            font-weight: 500;
            transition: all 0.2s ease;
            box-shadow: 0 4px 12px rgba(139, 92, 246, 0.3);
        }}
        
        .trace-link:hover {{ 
            background: var(--accent-hover);
            transform: translateY(-2px);
            box-shadow: 0 6px 20px rgba(139, 92, 246, 0.4);
        }}
        
        .test-image {{ 
            margin-top: 20px; 
            text-align: center;
            padding: 20px;
            background: var(--bg-tertiary);
            border-radius: 12px;
            border: 1px solid var(--border);
        }}
        
        .test-image img {{ 
            max-width: 100%; 
            border: 1px solid var(--border); 
            border-radius: 8px;
            box-shadow: 0 4px 12px var(--shadow);
        }}
        
        .timestamp {{ 
            color: var(--text-muted); 
            font-size: 0.9em; 
        }}
        
        .callgraph-toggle {{
            background: var(--bg-secondary);
            border: 1px solid var(--border);
            border-radius: 12px;
            padding: 20px;
            margin-bottom: 30px;
            box-shadow: 0 10px 40px var(--shadow);
        }}
        
        .callgraph-btn {{
            background: var(--bg-tertiary);
            color: var(--text-primary);
            border: 1px solid var(--border);
            padding: 12px 24px;
            border-radius: 8px;
            cursor: pointer;
            font-size: 1em;
            font-weight: 500;
            transition: all 0.2s ease;
        }}
        
        .callgraph-btn:hover {{
            background: var(--accent);
            color: white;
            border-color: var(--accent);
        }}
        
        .baseline-msg {{
            background: var(--success-bg);
            color: var(--success);
            padding: 12px 16px;
            border-radius: 8px;
            margin-top: 15px;
            border: 1px solid var(--success);
            font-weight: 500;
        }}
        
        a[href^="http"], a[href^="trace__"], a[href^="#"] {{
            color: var(--accent-primary);
            text-decoration: none;
            transition: color 0.2s ease;
        }}
        
        a[href^="http"]:hover, a[href^="trace__"]:hover {{
            color: var(--accent-hover-blue);
        }}
    </style>
</head>
<body>
    <div class="container">
        <h1>Render Test Report</h1>
        
        <div class="summary">
            <div class="header-info">
                <span class="timestamp">Generated: {self.generated_at}</span>
                <span>Tracing: {"Enabled" if self.tracing_enabled else "Disabled"}</span>
            </div>
            {baseline_msg}
            <div class="summary-stats">
                <div class="stat">
                    <div class="stat-value">{total}</div>
                    <div class="stat-label">Total Tests</div>
                </div>
                <div class="stat">
                    <div class="stat-value passed">{passed}</div>
                    <div class="stat-label">Passed</div>
                </div>
                <div class="stat">
                    <div class="stat-value failed">{failed}</div>
                    <div class="stat-label">Failed</div>
                </div>
                <div class="stat">
                    <div class="stat-value regression">{md5_regressions}</div>
                    <div class="stat-label">MD5 Regressions</div>
                </div>
            </div>
        </div>
        {callgraph_section}
        {results_html}
    </div>
</body>
</html>"""

    def _build_test_result(self, result: TestReportData) -> str:
        status_class = "passed" if result.status == "PASSED" else "failed"

        # Build MD5 comparison HTML
        if result.md5_hash:
            if result.previous_md5:
                if result.previous_md5 == result.md5_hash:
                    md5_html = (
                        f'<span class="md5-match">✅ {result.md5_hash} (match)</span>'
                    )
                else:
                    md5_html = f'<span class="md5-mismatch">⚠️ {result.md5_hash}<br>Previous: {result.previous_md5}</span>'
            else:
                md5_html = f'<span class="md5-no-baseline">➖ {result.md5_hash} (no baseline)</span>'
        else:
            md5_html = "N/A"

        # Build trace link HTML
        trace_html = ""
        if self.tracing_enabled and result.trace_html_filename:
            trace_html = f'<a class="trace-link" href="{result.trace_html_filename}" target="_blank">📊 View Call Tree</a>'

        # Build call graph links (from callgrind)
        cg_links = ""
        callgraphs = getattr(result, "callgraphs", [])
        for cg_html in callgraphs:
            cg_base = os.path.basename(cg_html)
            cg_links += (
                f'<a href="{cg_base}" target="_blank" '
                f'style="color:#79c0ff;font-size:12px;margin-left:12px;">View Call Graph</a>'
            )
        if self.viewer_base_url and callgraphs:
            test_slug = f"{result.test_suite}__{result.test_name}"
            viewer_url = f"{self.viewer_base_url.rstrip('/')}/graph/{test_slug}"
            cg_links += (
                f' <a href="{viewer_url}" target="_blank" '
                f'style="color:#79c0ff;font-size:12px;margin-left:12px;">View in Call Graph Viewer</a>'
            )

        # Build PNG image HTML
        img_html = ""
        if result.png_filename and os.path.exists(
            os.path.join(self.output_dir, result.png_filename)
        ):
            img_html = f"""
                <div class="test-image">
                    <img src="{result.png_filename}" alt="{result.test_name}">
                </div>"""
        else:
            # No PNG produced by this test
            img_html = """
                <div class="test-image" style="color:#8b949e; font-style:italic; padding:20px; text-align:center;">
                    No render output (test did not produce image)
                </div>"""

        # Build environment HTML
        env_items = [f"{k}={v}" for k, v in result.environment.items()]
        env_html = ", ".join(env_items) if env_items else "(none)"

        return f"""
        <div class="test-result">
            <div class="test-header">
                <span class="test-name">{result.test_suite}.{result.test_name}</span>
                <span class="test-status {status_class}">{result.status}</span>
            </div>
            <div class="test-details">
                <table class="detail-table">
                    <tr><th>Executable</th><td>{result.executable}</td></tr>
                    <tr><th>Test Suite</th><td>{result.test_suite}</td></tr>
                    <tr><th>Duration</th><td>{result.duration_ms}ms</td></tr>
                    <tr><th>MD5 Hash</th><td>{md5_html}</td></tr>
                    <tr><th>Environment</th><td>{env_html}</td></tr>
                </table>
                {trace_html}
                {cg_links}
                {img_html}
            </div>
        </div>"""


def generate(
    results: list,
    output_dir: str,
    baseline_dir: str = None,
    report_path: str = "render_report.html",
    update_baseline: bool = False,
    tracing_enabled: bool = True,
    callgraph_path: Optional[str] = None,
    viewer_base_url: Optional[str] = None,
) -> None:
    """
    Write render_report.html to report_path.

    Each TestResult contains:
        exe:    str        — executable name
        passed: bool       — GTest exit code
        pngs:   list[str]  — PNG file paths
        md5s:   list[str]  — MD5 file paths
        traces: list[str]  — trace JSON file paths (may be empty if tracing disabled)

    If callgraph_path is set, the report includes a "Call graph (this run)" section.
    If viewer_base_url is set, per-test "View in Call Graph Viewer" links point to the Next.js viewer.
    """
    generator = HTMLReportGenerator(
        output_dir,
        baseline_dir,
        update_baseline,
        callgraph_path=callgraph_path,
        viewer_base_url=viewer_base_url,
    )
    generator.set_tracing_enabled(tracing_enabled)

    # Process results
    for result in results:
        # Get test info from the result
        test_name = result.get("test_name", "Unknown")
        test_suite = result.get("test_suite", "Unknown")
        duration_ms = result.get("duration_ms", 0)
        status = "PASSED" if result.get("passed", False) else "FAILED"

        # Find PNG file - match against test name with new naming format
        # New format: Test{Suite}_{Test}_{custom}.png
        png_filename = None
        test_name = result.get("test_name", "")
        test_suite = result.get("test_suite", "")

        for png in result.get("pngs", []):
            basename = os.path.basename(png)
            # Match new format: Test{Suite}_{Test}_*.png
            if f"Test{test_suite}_{test_name}_" in basename:
                png_filename = basename
                break
            # Also try old patterns for backwards compatibility
            possible_patterns = [
                f"Test{test_name}.png",
                f"Test{test_name}Test.png",
                f"Test{test_name.replace('Render', '')}.png",
                f"Test{test_name.replace('Test', '')}.png",
            ]
            if basename in possible_patterns:
                png_filename = basename
                break

        # Find MD5 file and load baseline
        md5_hash = None
        for md5_file in result.get("md5s", []):
            with open(md5_file, "r") as f:
                md5_hash = f.read().strip()
            break

        # Get previous baseline
        previous_md5 = None
        if baseline_dir:
            baseline_md5_file = os.path.join(baseline_dir, f"Test{test_name}.md5")
            if os.path.exists(baseline_md5_file):
                with open(baseline_md5_file, "r") as f:
                    previous_md5 = f.read().strip()

        # Find trace HTML file
        trace_html_filename = None
        for trace in result.get("traces", []):
            trace_html_filename = os.path.basename(trace).replace(".json", ".html")
            break

        test_data = TestReportData(
            test_name=test_name,
            test_suite=test_suite,
            executable=result.get("exe", ""),
            duration_ms=duration_ms,
            md5_hash=md5_hash,
            previous_md5=previous_md5,
            status=status,
            output_text="",
            environment=result.get("environment", {}),
            render_functions=[],
            png_filename=png_filename or "",
            trace_html_filename=trace_html_filename,
            callgraphs=result.get("callgraphs", []),
        )
        generator.add_test_result(test_data)

    generator.generate(report_path)
