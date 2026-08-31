/**
 * Utilities for rendering React components to static HTML.
 *
 * @module utils/renderToHtml
 */

import React from 'react';
import {renderToString} from 'react-dom/server';
import {UnitReportPage} from '../components/UnitReportPage';
import {CombinedReportPage} from '../components/CombinedReportPage';
import type {TestReportData, UnitTestReportData} from '../types';

/**
 * CSS styles to inline in generated HTML.
 * This includes the Tailwind-generated styles and custom theme.
 */
const CRITICAL_CSS = `
  /* Base reset */
  *, *::before, *::after {
    box-sizing: border-box;
  }

  /* Dark theme CSS variables */
  :root {
    --color-bg-primary: #0f1117;
    --color-bg-secondary: #161b22;
    --color-bg-tertiary: #21262d;
    --color-text-primary: #f1f5f9;
    --color-text-secondary: #cbd5e1;
    --color-text-muted: #94a3b8;
    --color-accent: #8b5cf6;
    --color-accent-hover: #a78bfa;
    --color-accent-blue: #58a6ff;
    --color-accent-blue-hover: #79c0ff;
    --color-border: #2d3748;
    --color-success: #56d364;
    --color-success-bg: rgba(35, 134, 54, 0.2);
    --color-error: #ff7b72;
    --color-error-bg: rgba(218, 54, 51, 0.2);
    --color-warning: #d29922;
    --color-warning-bg: rgba(158, 106, 3, 0.2);
    --shadow-report: 0 10px 40px rgba(0, 0, 0, 0.4);
    --font-sans: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, Oxygen, Ubuntu, sans-serif;
    --font-mono: 'Menlo', 'Consolas', 'Monaco', monospace;
  }

  :root {
    color-scheme: dark;
  }

  body {
    font-family: var(--font-sans);
    line-height: 1.6;
    margin: 0;
    padding: 20px;
    background-color: var(--color-bg-primary);
    color: var(--color-text-primary);
  }

  .container {
    max-width: 1200px;
    margin: 0 auto;
  }

  h1 {
    color: var(--color-text-primary);
    border-bottom: 2px solid var(--color-accent);
    padding-bottom: 15px;
    font-size: 2.5em;
    font-weight: 700;
    margin-bottom: 30px;
  }

  a {
    color: var(--color-accent-blue);
    text-decoration: none;
    transition: color 0.2s ease;
  }

  a:hover {
    color: var(--color-accent-blue-hover);
  }

  .card {
    background-color: var(--color-bg-secondary);
    border: 1px solid var(--color-border);
    border-radius: 16px;
    box-shadow: var(--shadow-report);
    padding: 30px;
    margin-bottom: 25px;
  }

  .btn {
    display: inline-block;
    padding: 10px 20px;
    border-radius: 8px;
    font-weight: 500;
    transition: all 0.2s ease;
    cursor: pointer;
    border: none;
    font-size: 1rem;
    text-decoration: none;
  }

  .btn-primary {
    background-color: var(--color-accent);
    color: white;
    box-shadow: 0 4px 12px rgba(139, 92, 246, 0.3);
  }

  .btn-primary:hover {
    background-color: var(--color-accent-hover);
    transform: translateY(-2px);
    box-shadow: 0 6px 20px rgba(139, 92, 246, 0.4);
  }

  .detail-table {
    width: 100%;
    border-collapse: collapse;
    margin-bottom: 20px;
  }

  .detail-table th {
    color: var(--color-text-muted);
    font-weight: 500;
    width: 150px;
    text-transform: uppercase;
    font-size: 0.75rem;
    letter-spacing: 1px;
    padding: 12px 15px;
    text-align: left;
    border-bottom: 1px solid var(--color-border);
  }

  .detail-table td {
    color: var(--color-text-secondary);
    padding: 12px 15px;
    border-bottom: 1px solid var(--color-border);
  }

  .test-image {
    margin-top: 20px;
    text-align: center;
    padding: 20px;
    background-color: var(--color-bg-tertiary);
    border-radius: 12px;
    border: 1px solid var(--color-border);
  }

  .test-image img {
    max-width: 100%;
    border: 1px solid var(--color-border);
    border-radius: 8px;
    box-shadow: 0 4px 12px rgba(0, 0, 0, 0.4);
  }
`;

/**
 * Generates a simple HTML page for per-trace visualization.
 *
 * For now, this creates a placeholder that will be replaced with
 * the full D3.js tree view component.
 *
 * @param treeData - Tree data as JSON string
 * @param testName - Name of the test
 * @returns Complete HTML string
 */
export function generateTraceHtml(treeData: string, testName: string): string {
  const treeJsonSafe = treeData.replace(/<\/script>/g, '<\\/script>');
  
  return `<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Render Trace: ${testName}</title>
  <script src="https://cdnjs.cloudflare.com/ajax/libs/d3/7.8.5/d3.min.js"></script>
  <style>
    * { box-sizing: border-box; margin: 0; padding: 0; }
    body { font-family: 'Menlo','Consolas',monospace; background:#0d1117; color:#c9d1d9; padding:20px; }
    h1 { font-size:18px; margin-bottom:4px; color:#58a6ff; }
    .subtitle { font-size:12px; color:#8b949e; margin-bottom:20px; }
    #tree-controls { margin-bottom:12px; }
    button { background:#21262d; color:#c9d1d9; border:1px solid #30363d; padding:6px 14px;
              border-radius:6px; cursor:pointer; font-size:12px; margin-right:8px; }
    button:hover { background:#30363d; }
    #tree-container { background:#161b22; border:1px solid #30363d; border-radius:8px;
                        overflow:auto; padding:20px; min-height:400px; }
    .link { stroke:#30363d; stroke-width:1.5px; }
    .tooltip { position:fixed; background:#1c2128; border:1px solid #30363d; border-radius:6px;
                padding:10px 14px; font-size:11px; pointer-events:none; opacity:0;
                transition:opacity 0.15s; max-width:500px; word-break:break-all;
                z-index:100; color:#c9d1d9; }
    .tooltip .fn  { color:#79c0ff; font-weight:bold; margin-bottom:4px; }
    .tooltip .dur { color:#56d364; }
    .tooltip .addr { color:#8b949e; }
  </style>
</head>
<body>
  <h1>&#9654; Render Trace: ${testName}</h1>
  <div class="subtitle">Call tree visualization</div>
  <div id="tree-controls">
    <button onclick="resetZoom()">Reset Zoom</button>
  </div>
  <div id="tree-container"><svg id="tree-svg"></svg></div>
  <div class="tooltip" id="tooltip"></div>
<script>
const RAW_TREE = ${treeJsonSafe};
const COLORS = ['#58a6ff','#79c0ff','#56d364','#3fb950','#f0883e','#d29922','#bc8cff','#ff7b72'];
const colorFor = depth => COLORS[depth % COLORS.length];
const fmtDur = ns => ns < 1000 ? ns+'ns' : ns < 1e6 ? (ns/1000).toFixed(2)+'us'
                   : ns < 1e9  ? (ns/1e6).toFixed(2)+'ms' : (ns/1e9).toFixed(3)+'s';
const margin = {top:20,right:20,bottom:20,left:20};
const nodeH  = 28;
const NODE_W = 200;
const NODE_H = 80;
let nodeId = 0;

let svgTree, gTree, rootTree, zoomTree;
function initTree() {
  const container = document.getElementById('tree-container');
  const w = Math.max(container.clientWidth - 40, 900);
  svgTree = d3.select('#tree-svg').attr('width', w).attr('height', 600);
  zoomTree = d3.zoom().scaleExtent([0.2,4]).on('zoom', e => gTree.attr('transform', e.transform));
  svgTree.call(zoomTree);
  svgTree.selectAll('defs').remove();
  svgTree.append('defs').append('marker').attr('id','arrowhead').attr('viewBox','0 -5 10 10').attr('refX',10).attr('refY',0).attr('markerWidth',6).attr('markerHeight',6).attr('orient','auto').append('path').attr('d','M0,-5L10,0L0,5').attr('fill','#58a6ff');
  gTree = svgTree.append('g').attr('transform',\`translate(\${margin.left},\${margin.top})\`);
  rootTree = d3.hierarchy(RAW_TREE);
  
  // Calculate initial positions for ALL nodes
  d3.tree().nodeSize([100, 300])(rootTree);
  // Save these as base positions for all nodes
  rootTree.each(d => { d.baseX = d.x; d.baseY = d.y; d.savedX = d.x; d.savedY = d.y; });
  
  rootTree.x0 = rootTree.y0 = 0;
  // Don't collapse anything - show full tree
  updateTree(rootTree);
}

const resetZoom   = () => svgTree.transition().duration(400).call(zoomTree.transform, d3.zoomIdentity.translate(margin.left, margin.top));

function updateLinksTree() {
  const links = rootTree.links();
  const lk = gTree.selectAll('line.link').data(links, d => d.target.id);
  const lkE = lk.enter().insert('line','g').attr('class','link').attr('marker-end','url(#arrowhead)')
    .attr('x1', d => d.source.y + NODE_W/2)
    .attr('y1', d => d.source.x)
    .attr('x2', d => d.source.y + NODE_W/2)
    .attr('y2', d => d.source.x);
  lkE.merge(lk)
    .attr('x1', d => d.source.y + NODE_W/2)
    .attr('y1', d => d.source.x)
    .attr('x2', d => d.target.y - NODE_W/2 + 10)  // Extend line 10px into node so arrow tip lands on edge
    .attr('y2', d => d.target.x);
  lk.exit().remove();
}

function updateTree(src) {
  // Get all currently visible descendants
  const nodes = rootTree.descendants();
  const links = rootTree.links();
  
  // For newly visible nodes (entering), assign positions if they don't have saved positions
  nodes.forEach((d, i) => {
    // If this node doesn't have a saved position yet, use its base position or place relative to parent
    if (d.savedX === undefined || d.savedY === undefined) {
      if (d.parent) {
        // Place relative to parent with offset based on sibling index
        const siblings = d.parent.children || [];
        const idx = siblings.indexOf(d);
        d.savedY = d.parent.savedY + 300;  // 300px to the right of parent
        d.savedX = d.parent.savedX + (idx * 100);  // 100px spacing between siblings
      } else {
        // Root node - use base position
        d.savedX = d.baseX || 0;
        d.savedY = d.baseY || 0;
      }
    }
    // Set current x/y from saved positions
    d.x = d.savedX;
    d.y = d.savedY;
  });
  
  let nc = nodes.length;
  svgTree.attr('height', Math.max(600, nc * nodeH + margin.top + margin.bottom));
  
  const nd = gTree.selectAll('g.node').data(nodes, d => d.id || (d.id = ++nodeId));
  const ndE = nd.enter().append('g').attr('class','node')
    .attr('transform', d => \`translate(\${d.y},\${d.x})\`)
    .on('mouseover', showTooltip).on('mousemove', moveTooltip).on('mouseout', hideTooltip);
  const card = ndE.append('foreignObject').attr('width',NODE_W).attr('height',NODE_H).attr('x',-NODE_W/2).attr('y',-NODE_H/2).append('xhtml:div')
    .style('width', NODE_W+'px').style('height',NODE_H+'px')
    .style('background', d => d.data.endpoint ? '#2d1a0e' : '#161b22')
    .style('border', d => d.data.endpoint ? '2px solid #f0883e' : '2px solid ' + colorFor(d.depth||0))
    .style('border-radius','6px').style('padding','6px 10px').style('font-family','Menlo, Consolas, monospace').style('font-size','11px').style('color','#c9d1d9').style('cursor','pointer')
    .html(d => {
      const fn = d.data.fn||'??'; const title = fn.length>24 ? fn.slice(0,22)+'...' : fn;
      const calls = d.data.calls||1; const total = d.data.total_time_ns != null ? d.data.total_time_ns : (d.data.duration||0);
      const avg = d.data.avg_time_ns != null ? d.data.avg_time_ns : (d.data.duration||0);
      const hiddenChildren = (!d.children && d._children) ? (d.data.child_count||d._children.length||0) : 0;
      return \`<div style="color:\${d.data.endpoint?'#f0883e':'#79c0ff'};font-weight:bold;">\${title}</div><div style="border-top:1px solid #30363d;margin:4px 0;"></div><div style="color:#8b949e;">calls: <span style="color:#c9d1d9;">\${calls}</span> time: <span style="color:#56d364;">\${fmtDur(total)}</span></div>\${hiddenChildren ? '<div style="color:#8b949e;">+'+hiddenChildren+' children</div>' : ''}\`;
    });
  ndE.call(d3.drag()
    .on('start', function(e, d) {
      d3.select(this).raise();
      // Store the grab offset: where on the node we clicked
      d.grabOffsetX = e.x - d.y;
      d.grabOffsetY = e.y - d.x;
    })
    .on('drag', function(e, d) {
      // Apply grab offset so node follows cursor from where we grabbed it
      d.y = e.x - d.grabOffsetX;
      d.x = e.y - d.grabOffsetY;
      d.savedY = d.y;  // Save the new position
      d.savedX = d.x;
      d3.select(this).attr('transform', \`translate(\${d.y},\${d.x})\`);
      updateLinksTree();
    })
    .on('end', function(e, d) {
      // Distinguish click from drag
      const moved = Math.abs(d.x - (e.y - d.grabOffsetY)) + Math.abs(d.y - (e.x - d.grabOffsetX));
      if (moved < 10) {
        // It's a click - expand/collapse
        if (d.children) { d._children = d.children; d.children = null; }
        else if (d._children) { d.children = d._children; d._children = null; }
        updateTree(d);
      }
    }));
  const ndU = ndE.merge(nd);
  ndU.attr('transform', d => \`translate(\${d.y},\${d.x})\`);
  nd.exit().transition().duration(250).remove();
  updateLinksTree();
}

const tipEl = document.getElementById('tooltip');
function showTooltip(ev, d) {
  const data = d.data || d;
  const fn = data.fn || data.label || d.id || '??';
  const dur = data.duration != null ? data.duration : (data.total_time_ns != null ? data.total_time_ns : 0);
  tipEl.innerHTML = \`<div class="fn">\${fn}</div><div class="dur">\${fmtDur(dur)}</div>\`;
  tipEl.style.opacity = '1'; moveTooltip(ev);
}
function moveTooltip(ev) { tipEl.style.left=(ev.clientX+16)+'px'; tipEl.style.top=(ev.clientY+16)+'px'; }
function hideTooltip() { tipEl.style.opacity='0'; }

initTree();
</script>
</body>
</html>`;
}

/**
 * Generates the unit test report HTML (non-render tests).
 *
 * @param data - Unit report data (summary + results)
 * @returns Complete HTML string
 */
export function generateUnitReportHtml(data: {
  generatedAt: string;
  total: number;
  passed: number;
  failed: number;
  results: UnitTestReportData[];
}): string {
  const htmlContent = renderToString(React.createElement(UnitReportPage, data));

  return `<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Unit Test Report</title>
  <style>${CRITICAL_CSS}</style>
</head>
<body>
  ${htmlContent}
</body>
</html>`;
}

/**
 * Generates the combined test report HTML with Unit and Render tabs.
 *
 * @param data - Combined report data (both unit and render)
 * @returns Complete HTML string
 */
export function generateCombinedReportHtml(data: {
  generatedAt: string;
  unitTotal: number;
  unitPassed: number;
  unitFailed: number;
  unitResults: UnitTestReportData[];
  renderTotal: number;
  renderPassed: number;
  renderFailed: number;
  renderResults: TestReportData[];
  tracingEnabled: boolean;
  md5Regressions: number;
}): string {
  const htmlContent = renderToString(React.createElement(CombinedReportPage, data));

  const TAB_JS = `
<script>
(function() {
  function showTab(tabName) {
    // Update buttons
    document.querySelectorAll('.tab-button').forEach(function(btn) {
      if (btn.dataset.tab === tabName) {
        btn.classList.add('active');
        btn.style.backgroundColor = 'var(--color-bg-secondary)';
        btn.style.borderBottom = '2px solid var(--color-accent)';
        btn.style.color = 'var(--color-text-primary)';
      } else {
        btn.classList.remove('active');
        btn.style.backgroundColor = 'transparent';
        btn.style.borderBottom = '2px solid transparent';
        btn.style.color = 'var(--color-text-muted)';
      }
    });
    
    // Update content
    document.querySelectorAll('.tab-content').forEach(function(content) {
      if (content.id === 'content-' + tabName) {
        content.classList.add('active');
        content.style.display = 'block';
      } else {
        content.classList.remove('active');
        content.style.display = 'none';
      }
    });
  }
  
  document.addEventListener('DOMContentLoaded', function() {
    document.querySelectorAll('.tab-button').forEach(function(btn) {
      btn.addEventListener('click', function() {
        showTab(this.dataset.tab);
      });
    });
  });
})();
</script>`;

  const TRACE_MODAL_JS = `
<script src="https://cdnjs.cloudflare.com/ajax/libs/d3/7.8.5/d3.min.js"></script>
<script>
(function() {
  const COLORS = ['#58a6ff','#79c0ff','#56d364','#3fb950','#f0883e','#d29922','#bc8cff','#ff7b72'];
  const colorFor = depth => COLORS[depth % COLORS.length];
  const fmtDur = ns => ns < 1000 ? ns+'ns' : ns < 1e6 ? (ns/1000).toFixed(2)+'us' : ns < 1e9 ? (ns/1e6).toFixed(2)+'ms' : (ns/1e9).toFixed(3)+'s';
  const NODE_W = 200;
  const NODE_H = 80;
  let nodeId = 0;
  let currentSvg, currentG, currentRoot, currentZoom;
  let currentTipEl;

  function initTraceModal(traceData, testName) {
    const container = document.getElementById('modal-tree-container');
    const width = container.clientWidth - 40;
    
    // Clear any existing content
    d3.select('#modal-tree-svg').selectAll('*').remove();
    
    currentSvg = d3.select('#modal-tree-svg').attr('width', width).attr('height', 500);
    currentZoom = d3.zoom().scaleExtent([0.2, 4]).on('zoom', e => currentG.attr('transform', e.transform));
    currentSvg.call(currentZoom);
    currentSvg.append('defs').append('marker').attr('id','arrowhead').attr('viewBox','0 -5 10 10').attr('refX',10).attr('refY',0).attr('markerWidth',6).attr('markerHeight',6).attr('orient','auto').append('path').attr('d','M0,-5L10,0L0,5').attr('fill','#58a6ff');
    currentG = currentSvg.append('g').attr('transform', 'translate(40,40)');
    
    // Reset node ID counter for fresh tree
    nodeId = 0;
    
    // Build tree from trace events
    const tree = buildTree(traceData);
    currentRoot = d3.hierarchy(tree);
    
    // Calculate initial positions
    d3.tree().nodeSize([100, 300])(currentRoot);
    currentRoot.each(d => { d.baseX = d.x; d.baseY = d.y; d.savedX = d.x; d.savedY = d.y; });
    currentRoot.x0 = currentRoot.y0 = 0;
    
    // Create tooltip if not exists
    if (!currentTipEl) {
      currentTipEl = document.createElement('div');
      currentTipEl.className = 'tooltip';
      currentTipEl.style.cssText = 'position:fixed;background:#1c2128;border:1px solid #30363d;border-radius:6px;padding:10px 14px;font-size:11px;pointer-events:none;opacity:0;transition:opacity 0.15s;max-width:500px;word-break:break-all;z-index:1002;color:#c9d1d9;';
      document.body.appendChild(currentTipEl);
    }
    
    updateModalTree(currentRoot);
  }
  
  function buildTree(events) {
    // Patterns to filter out (C++ stdlib and testing framework noise)
    const FILTER_PATTERNS = [
      /^testing::/,           // Google Test framework
      /^std::/,              // C++ standard library
      /_Test::_Test\\(/,      // Test class constructors
      /_Test::~_Test\\(/,     // Test class destructors
      /^_Test::/,            // Test class methods
      /\\b__.*\\b/,            // Compiler internals (starts with __)
    ];
    
    function shouldFilter(fn) {
      if (!fn || fn === '??') return false;
      return FILTER_PATTERNS.some(pattern => pattern.test(fn));
    }
    
    const root = {fn: 'root', ts: 0, duration: 0, children: []};
    const stack = [root];
    
    for (const event of events) {
      if (event.type === 'enter') {
        const fn = event.fn || '??';
        
        // Skip filtered functions - don't add to tree
        if (shouldFilter(fn)) {
          // Still push to stack so we can match the exit
          stack.push({filtered: true, fn});
          continue;
        }
        
        const node = {
          fn: fn,
          addr: event.addr || '',
          ts: event.ts_ns,
          duration: 0,
          children: []
        };
        
        // Find the last non-filtered parent
        let parentIdx = stack.length - 1;
        while (parentIdx >= 0 && stack[parentIdx].filtered) {
          parentIdx--;
        }
        
        if (parentIdx >= 0) {
          stack[parentIdx].children.push(node);
        }
        stack.push(node);
      } else if (event.type === 'exit' && stack.length > 1) {
        const node = stack.pop();
        if (!node.filtered) {
          node.duration = event.ts_ns - node.ts;
        }
      }
    }
    
    // Annotate stats
    function annotateStats(node) {
      const children = node.children || [];
      node.child_count = children.length;
      node.calls = node.calls || 1;
      node.total_time_ns = node.duration || 0;
      node.avg_time_ns = node.total_time_ns / (node.calls || 1);
      for (const child of children) annotateStats(child);
    }
    annotateStats(root);
    
    return root;
  }
  
  function updateLinksTree() {
    const links = currentRoot.links();
    const lk = currentG.selectAll('line.link').data(links, d => d.target.id);
    const lkE = lk.enter().insert('line','g').attr('class','link').attr('marker-end','url(#arrowhead)')
      .attr('x1', d => d.source.y + NODE_W/2)
      .attr('y1', d => d.source.x)
      .attr('x2', d => d.source.y + NODE_W/2)
      .attr('y2', d => d.source.x)
      .style('stroke', '#30363d').style('stroke-width', 1.5);
    lkE.merge(lk)
      .attr('x1', d => d.source.y + NODE_W/2)
      .attr('y1', d => d.source.x)
      .attr('x2', d => d.target.y - NODE_W/2 + 10)
      .attr('y2', d => d.target.x);
    lk.exit().remove();
  }
  
  function updateModalTree(src) {
    const nodes = currentRoot.descendants();
    nodes.forEach(d => {
      if (d.savedX === undefined || d.savedY === undefined) {
        if (d.parent) {
          const siblings = d.parent.children || [];
          const idx = siblings.indexOf(d);
          d.savedY = d.parent.savedY + 300;
          d.savedX = d.parent.savedX + (idx * 100);
        } else {
          d.savedX = d.baseX || 0;
          d.savedY = d.baseY || 0;
        }
      }
      d.x = d.savedX;
      d.y = d.savedY;
    });
    
    currentSvg.attr('height', Math.max(500, nodes.length * 28 + 80));
    
    const nd = currentG.selectAll('g.node').data(nodes, d => d.id || (d.id = ++nodeId));
    const ndE = nd.enter().append('g').attr('class','node')
      .attr('transform', d => 'translate(' + d.y + ',' + d.x + ')')
      .on('mouseover', function(e, d) {
        const data = d.data || d;
        const fn = data.fn || '??';
        const dur = data.duration != null ? data.duration : (data.total_time_ns || 0);
        currentTipEl.innerHTML = '<div style="color:#79c0ff;font-weight:bold;">' + fn + '</div><div style="color:#56d364;">' + fmtDur(dur) + '</div>';
        currentTipEl.style.opacity = '1';
        currentTipEl.style.left = (e.clientX + 16) + 'px';
        currentTipEl.style.top = (e.clientY + 16) + 'px';
      })
      .on('mousemove', function(e) {
        currentTipEl.style.left = (e.clientX + 16) + 'px';
        currentTipEl.style.top = (e.clientY + 16) + 'px';
      })
      .on('mouseout', function() {
        currentTipEl.style.opacity = '0';
      });
      
    ndE.append('foreignObject').attr('width',NODE_W).attr('height',NODE_H).attr('x',-NODE_W/2).attr('y',-NODE_H/2).append('xhtml:div')
      .style('width', NODE_W+'px').style('height',NODE_H+'px')
      .style('background', d => d.data.endpoint ? '#2d1a0e' : '#161b22')
      .style('border', d => d.data.endpoint ? '2px solid #f0883e' : '2px solid ' + colorFor(d.depth||0))
      .style('border-radius','6px').style('padding','6px 10px').style('font-family','Menlo, Consolas, monospace').style('font-size','11px').style('color','#c9d1d9').style('cursor','pointer')
      .html(d => {
        const fn = d.data.fn||'??'; const title = fn.length>24 ? fn.slice(0,22)+'...' : fn;
        const calls = d.data.calls||1; const total = d.data.total_time_ns != null ? d.data.total_time_ns : (d.data.duration||0);
        const hiddenChildren = (!d.children && d._children) ? (d.data.child_count||d._children.length||0) : 0;
        return '<div style="color:' + (d.data.endpoint?'#f0883e':'#79c0ff') + ';font-weight:bold;">' + title + '</div><div style="border-top:1px solid #30363d;margin:4px 0;"></div><div style="color:#8b949e;">calls: <span style="color:#c9d1d9;">' + calls + '</span> time: <span style="color:#56d364;">' + fmtDur(total) + '</span></div>' + (hiddenChildren ? '<div style="color:#8b949e;">+'+hiddenChildren+' children</div>' : '');
      });
      
    ndE.call(d3.drag()
      .on('start', function(e, d) {
        d3.select(this).raise();
        d.grabOffsetX = e.x - d.y;
        d.grabOffsetY = e.y - d.x;
      })
      .on('drag', function(e, d) {
        d.y = e.x - d.grabOffsetX;
        d.x = e.y - d.grabOffsetY;
        d.savedY = d.y;
        d.savedX = d.x;
        d3.select(this).attr('transform', 'translate(' + d.y + ',' + d.x + ')');
        updateLinksTree();
      })
      .on('end', function(e, d) {
        const moved = Math.abs(d.x - (e.y - d.grabOffsetY)) + Math.abs(d.y - (e.x - d.grabOffsetX));
        if (moved < 10) {
          if (d.children) { d._children = d.children; d.children = null; }
          else if (d._children) { d.children = d._children; d._children = null; }
          updateModalTree(d);
        }
      }));
      
    ndE.merge(nd).attr('transform', d => 'translate(' + d.y + ',' + d.x + ')');
    nd.exit().remove();
    updateLinksTree();
  }
  
  function resetZoom() {
    if (currentSvg && currentZoom) {
      currentSvg.transition().duration(400).call(currentZoom.transform, d3.zoomIdentity.translate(40, 40));
    }
  }
  
  // Setup event listeners
  document.addEventListener('DOMContentLoaded', function() {
    // Trace button clicks
    document.querySelectorAll('.trace-btn').forEach(function(btn) {
      btn.addEventListener('click', function() {
        const traceData = JSON.parse(this.dataset.trace.replace(/&quot;/g, '"'));
        const testName = this.dataset.testName;
        document.getElementById('modal-title').textContent = 'Trace: ' + testName;
        document.getElementById('trace-modal').style.display = 'block';
        initTraceModal(traceData, testName);
      });
    });
    
    // Close modal
    document.getElementById('modal-close').addEventListener('click', function() {
      document.getElementById('trace-modal').style.display = 'none';
    });
    
    // Close on backdrop click
    document.querySelector('.modal-backdrop').addEventListener('click', function() {
      document.getElementById('trace-modal').style.display = 'none';
    });
    
    // Reset zoom button
    document.getElementById('reset-zoom-btn').addEventListener('click', resetZoom);
    
    // Escape key to close
    document.addEventListener('keydown', function(e) {
      if (e.key === 'Escape') {
        document.getElementById('trace-modal').style.display = 'none';
      }
    });
  });
})();
</script>`;

  return `<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Test Report</title>
  <style>${CRITICAL_CSS}</style>
</head>
<body>
  ${htmlContent}
  ${TAB_JS}
  ${TRACE_MODAL_JS}
</body>
</html>`;
}
