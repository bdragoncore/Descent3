import type { CallGraph, LayoutNode } from "./types";

const NODE_W = 230;
const NODE_H = 96;
const COL_GAP = 120;
const ROW_GAP = 30;

const DEPTH_COLORS = [
  "#58a6ff",
  "#79c0ff",
  "#56d364",
  "#3fb950",
  "#d29922",
  "#bc8cff",
  "#ff7b72",
];

function getDepthColor(depth: number): string {
  return DEPTH_COLORS[depth % DEPTH_COLORS.length];
}

export function getNodeColor(depth: number, endpoint: boolean): string {
  if (endpoint) return "#f0883e";
  return getDepthColor(depth);
}

export interface LayoutResult {
  nodes: LayoutNode[];
  width: number;
  height: number;
  nodeWidth: number;
  nodeHeight: number;
  colGap: number;
  rowGap: number;
}

/**
 * BFS layout for the call graph. Places nodes in columns by call depth.
 * Accepts both raw and filtered/grouped graphs (grouped nodes use [brackets]).
 */
export function computeLayout(graph: CallGraph): LayoutResult {
  const { root, nodes, edges } = graph;
  const nodeMap = new Map<string, LayoutNode>();

  // BFS to assign depth
  const depth = new Map<string, number>();
  const adj: Map<string, string[]> = new Map();
  for (const e of edges) {
    if (!adj.has(e.from)) adj.set(e.from, []);
    adj.get(e.from)!.push(e.to);
  }
  const queue: string[] = [root];
  depth.set(root, 0);
  while (queue.length > 0) {
    const u = queue.shift()!;
    const d = depth.get(u)!;
    for (const v of adj.get(u) ?? []) {
      if (!depth.has(v)) {
        depth.set(v, d + 1);
        queue.push(v);
      }
    }
  }

  // Include root and any node that appears in edges
  const allIds = new Set<string>([root]);
  for (const e of edges) {
    allIds.add(e.from);
    allIds.add(e.to);
  }

  // Group by depth
  const byDepth = new Map<number, string[]>();
  for (const id of Array.from(allIds)) {
    const d = depth.get(id) ?? 0;
    if (!byDepth.has(d)) byDepth.set(d, []);
    byDepth.get(d)!.push(id);
  }

  const maxDepth = Math.max(...byDepth.keys(), 0);
  let maxRows = 0;
  for (const [, ids] of byDepth) {
    if (ids.length > maxRows) maxRows = ids.length;
  }

  const startX = 40;
  const startY = 40;

  for (const [d, ids] of byDepth) {
    const colX = startX + d * (NODE_W + COL_GAP);
    const totalHeight = ids.length * NODE_H + (ids.length - 1) * ROW_GAP;
    let y = startY + Math.max(0, (maxRows * (NODE_H + ROW_GAP) - totalHeight) / 2);
    for (const id of ids) {
      const data = nodes[id];
      if (!data) continue;
      // Detect grouped nodes: show module name without brackets as label
      const label = id.startsWith("[") && id.endsWith("]")
        ? id.slice(1, -1)
        : data.short_fn || id;
      nodeMap.set(id, {
        ...data,
        id,
        x: colX,
        y,
        depth: d,
        short_fn: label,
      });
      y += NODE_H + ROW_GAP;
    }
  }

  const width = startX + (maxDepth + 1) * (NODE_W + COL_GAP) + 40;
  const height = startY + maxRows * (NODE_H + ROW_GAP) + 40;

  return {
    nodes: Array.from(nodeMap.values()),
    width,
    height,
    nodeWidth: NODE_W,
    nodeHeight: NODE_H,
    colGap: COL_GAP,
    rowGap: ROW_GAP,
  };
}
