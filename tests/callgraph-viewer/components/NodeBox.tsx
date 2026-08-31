"use client";

import type { LayoutNode } from "@/lib/types";
import { getNodeColor } from "@/lib/layout";
import { getModuleColor } from "@/lib/filter";
import { classifyFn } from "@/lib/filter";

const NODE_W = 230;
const NODE_H = 96;

interface NodeBoxProps {
  node: LayoutNode;
  onDrag?: (dx: number, dy: number) => void;
}

export function NodeBox({ node, onDrag }: NodeBoxProps) {
  const isGrouped = node.id.startsWith("[") && node.id.endsWith("]");
  const moduleColor = isGrouped
    ? getModuleColor(node.id)
    : getNodeColor(node.depth, node.endpoint);
  const borderColor = isGrouped ? moduleColor : getNodeColor(node.depth, node.endpoint);
  const headerBg = isGrouped ? `${moduleColor}18` : node.endpoint ? "#2d1a0e" : "#1c2128";
  const fnColor = isGrouped ? moduleColor : node.endpoint ? "#f0883e" : "#79c0ff";

  const formatCost = (n: number) =>
    n >= 1000 ? `${(n / 1000).toFixed(1)}K` : String(n);

  return (
    <>
      <g
        transform={`translate(${node.x}, ${node.y})`}
        style={{ cursor: onDrag ? "move" : "default" }}
      >
      <rect
        x={0}
        y={0}
        width={NODE_W}
        height={NODE_H}
        fill="#161b22"
        stroke={borderColor}
        strokeWidth={2}
        rx={4}
      />
      <rect
        x={2}
        y={2}
        width={NODE_W - 4}
        height={24}
        fill={headerBg}
        rx={2}
      />
      <text
        x={8}
        y={18}
        fill={fnColor}
        fontSize={12}
        fontFamily="monospace"
        style={{ overflow: "hidden", textOverflow: "ellipsis" }}
      >
        {node.short_fn}
      </text>
      <line
        x1={4}
        y1={26}
        x2={NODE_W - 4}
        y2={26}
        stroke="#30363d"
        strokeWidth={1}
      />
      <text x={8} y={44} fill="#8b949e" fontSize={10}>
        calls in:
      </text>
      <text x={80} y={44} fill="#c9d1d9" fontSize={10}>
        {node.calls_in}
      </text>
      <text x={8} y={58} fill="#8b949e" fontSize={10}>
        self cost:
      </text>
      <text x={80} y={58} fill="#56d364" fontSize={10}>
        {formatCost(node.self_cost)} ir
      </text>
      <text x={8} y={72} fill="#8b949e" fontSize={10}>
        total cost:
      </text>
      <text x={80} y={72} fill="#56d364" fontSize={10}>
        {formatCost(node.total_cost)} ir
      </text>
      <text
        x={8}
        y={86}
        fill="#8b949e"
        fontSize={9}
        style={{ overflow: "hidden", textOverflow: "ellipsis" }}
      >
        {node.file}
      </text>
      {node.endpoint && (
        <text x={NODE_W - 70} y={20} fill="#f0883e" fontSize={9} fontWeight="bold">
          [ ENDPOINT ]
        </text>
      )}
      </g>
    </>
  );
}
