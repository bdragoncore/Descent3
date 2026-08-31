"use client";

import { useCallback, useEffect, useRef, useState } from "react";
import * as d3 from "d3";
import type { CallGraph as CallGraphData, LayoutNode } from "@/lib/types";
import { computeLayout } from "@/lib/layout";
import { filterGraph, type FilterOptions } from "@/lib/filter";
import { NodeBox } from "./NodeBox";
import { EdgeArrow } from "./EdgeArrow";
import { Toolbar } from "./Toolbar";
import { ModuleLegend } from "./ModuleLegend";
import { Tooltip } from "./Tooltip";

const NODE_W = 230;
const NODE_H = 96;

type TooltipState = {
  x: number;
  y: number;
  title: string;
  content: string;
} | null;

interface CallGraphProps {
  graph: CallGraphData;
}

const DEFAULT_FILTER: FilterOptions = {
  hideStdlib: true,
  hideMesaInternal: true,
  rootFn: "main",
  leafFn: "glFlush",
  maxDepth: 10,
  minCostPct: 0.001,
  enabledModules: new Set(),
  groupByModule: true,
};

export function CallGraph({ graph }: CallGraphProps) {
  const [filterOpts, setFilterOpts] = useState<FilterOptions>(DEFAULT_FILTER);
  const [enabledModules, setEnabledModules] = useState<Set<string>>(new Set());
  const [showLegend, setShowLegend] = useState(true);

  // Apply filter
  const filtered = filterGraph(graph, {
    ...filterOpts,
    enabledModules,
  });

  const layoutResult = computeLayout(filtered.graph);
  const [nodes, setNodes] = useState<LayoutNode[]>(layoutResult.nodes);
  const [transform, setTransform] = useState({ x: 0, y: 0, k: 1 });
  const [tooltip, setTooltip] = useState<TooltipState>(null);
  const svgRef = useRef<SVGSVGElement>(null);
  const innerRef = useRef<SVGGElement>(null);

  // Reset nodes when any filter changes or graph changes
  useEffect(() => {
    const fresh = computeLayout(filtered.graph);
    setNodes(fresh.nodes);
  }, [filtered.graph, filtered.graph.root, filtered.graph.nodes, filtered.graph.edges.length]);

  const zoomRef = useRef<d3.ZoomBehavior<SVGSVGElement, unknown> | null>(null);

  const applyZoomTransform = useCallback((x: number, y: number, k: number) => {
    const svg = svgRef.current;
    const zoom = zoomRef.current;
    if (svg && zoom) {
      zoom.transform(d3.select(svg), d3.zoomIdentity.translate(x, y).scale(k));
    }
  }, []);

  const handleResetLayout = useCallback(() => {
    setNodes(computeLayout(filtered.graph).nodes);
    setTransform({ x: 0, y: 0, k: 1 });
    setTimeout(() => applyZoomTransform(0, 0, 1), 0);
  }, [filtered.graph, applyZoomTransform]);

  const handleFitView = useCallback(() => {
    const w = layoutResult.width;
    const h = layoutResult.height;
    const container = svgRef.current?.parentElement;
    if (!container || w === 0 || h === 0) return;
    const cw = container.clientWidth;
    const ch = container.clientHeight;
    const k = Math.min(cw / w, ch / h, 2) * 0.9;
    const x = (cw - w * k) / 2;
    const y = (ch - h * k) / 2;
    setTransform({ x, y, k });
    setTimeout(() => applyZoomTransform(x, y, k), 0);
  }, [layoutResult.width, layoutResult.height, applyZoomTransform]);

  useEffect(() => {
    const svg = svgRef.current;
    if (!svg) return;
    const zoom = d3.zoom<SVGSVGElement, unknown>()
      .scaleExtent([0.1, 10])
      .on("zoom", (event) => {
        setTransform({ x: event.transform.x, y: event.transform.y, k: event.transform.k });
      });
    zoomRef.current = zoom;
    d3.select(svg).call(zoom);
    return () => {
      zoomRef.current = null;
      d3.select(svg).on(".zoom", null);
    };
  }, []);

  const handleNodeDrag = useCallback((id: string, dx: number, dy: number) => {
    setNodes((prev) =>
      prev.map((n) => (n.id === id ? { ...n, x: n.x + dx, y: n.y + dy } : n))
    );
  }, []);

  const handleFilterChange = useCallback((updates: Partial<FilterOptions>) => {
    setFilterOpts((prev) => ({ ...prev, ...updates }));
  }, []);

  const handleModuleToggle = useCallback((moduleName: string) => {
    setEnabledModules((prev) => {
      const next = new Set(prev);
      if (moduleName === "__all__") {
        return new Set();
      }
      if (next.has(moduleName)) {
        next.delete(moduleName);
      } else {
        next.add(moduleName);
      }
      return next;
    });
  }, []);

  const nodeMap = new Map(nodes.map((n) => [n.id, n]));

  return (
    <div>
      <Toolbar
        onResetLayout={handleResetLayout}
        onFitView={handleFitView}
        filterOptions={filterOpts}
        onFilterChange={handleFilterChange}
        report={filtered.report}
      />
      {showLegend && (
        <ModuleLegend enabledModules={enabledModules} onToggle={handleModuleToggle} />
      )}
      <div
        style={{
          width: "100%",
          height: "80vh",
          minHeight: 400,
          background: "#161b22",
          border: "1px solid #30363d",
          borderRadius: 8,
          overflow: "hidden",
        }}
      >
        <svg
          ref={svgRef}
          width="100%"
          height="100%"
          style={{ cursor: "grab" }}
        >
          <g
            ref={innerRef}
            transform={`translate(${transform.x},${transform.y}) scale(${transform.k})`}
          >
            {filtered.graph.edges.map((e, i) => {
              const src = nodeMap.get(e.from);
              const tgt = nodeMap.get(e.to);
              if (!src || !tgt) return null;
              return (
                <EdgeArrow
                  key={`${e.from}-${e.to}-${i}`}
                  x1={src.x}
                  y1={src.y}
                  x2={tgt.x}
                  y2={tgt.y}
                  sourceW={NODE_W}
                  sourceH={NODE_H}
                  targetW={NODE_W}
                  targetH={NODE_H}
                  cost={e.cost}
                  calls={e.calls}
                />
              );
            })}
            {nodes.map((node) => (
              <DraggableNode
                key={node.id}
                node={node}
                onDrag={(dx, dy) => handleNodeDrag(node.id, dx, dy)}
                onTooltip={setTooltip}
              />
            ))}
          </g>
        </svg>
      </div>
      <Tooltip
        x={tooltip?.x ?? 0}
        y={tooltip?.y ?? 0}
        title={tooltip?.title ?? ""}
        content={tooltip?.content}
        visible={!!tooltip}
      />
    </div>
  );
}

// ── DraggableNode ───────────────────────────────────────────────────────

interface DraggableNodeProps {
  node: LayoutNode;
  onDrag: (dx: number, dy: number) => void;
  onTooltip: React.Dispatch<React.SetStateAction<TooltipState>>;
}

function DraggableNode({ node, onDrag, onTooltip }: DraggableNodeProps) {
  const startRef = useRef<{ x: number; y: number } | null>(null);
  const onDragRef = useRef(onDrag);
  onDragRef.current = onDrag;

  const onMouseDown = useCallback((e: React.MouseEvent) => {
    e.preventDefault();
    startRef.current = { x: e.clientX, y: e.clientY };
    const onMove = (ev: MouseEvent) => {
      const start = startRef.current;
      if (!start) return;
      const dx = ev.clientX - start.x;
      const dy = ev.clientY - start.y;
      onDragRef.current(dx, dy);
      startRef.current = { x: ev.clientX, y: ev.clientY };
    };
    const onUp = () => {
      startRef.current = null;
      window.removeEventListener("mousemove", onMove);
      window.removeEventListener("mouseup", onUp);
    };
    window.addEventListener("mousemove", onMove);
    window.addEventListener("mouseup", onUp);
  }, []);

  return (
    <g
      onMouseDown={onMouseDown}
      onMouseEnter={(e) =>
        onTooltip({ x: e.clientX, y: e.clientY, title: node.fn, content: node.file })
      }
      onMouseLeave={() => onTooltip(null)}
      onMouseMove={(e) =>
        onTooltip((prev) =>
          prev ? { ...prev, x: e.clientX, y: e.clientY } : null
        )
      }
    >
      <NodeBox node={node} />
    </g>
  );
}
