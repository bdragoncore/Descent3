"use client";

interface EdgeArrowProps {
  x1: number;
  y1: number;
  x2: number;
  y2: number;
  sourceW: number;
  sourceH: number;
  targetW: number;
  targetH: number;
  /** Optional cost (e.g. time) for stroke width and tooltip */
  cost?: number;
  /** Optional call count for tooltip and fallback stroke width */
  calls?: number;
}

/**
 * Cubic bezier from right-center of source to left-center of target, with arrowhead.
 */
export function EdgeArrow({
  x1,
  y1,
  x2,
  y2,
  sourceW,
  sourceH,
  targetW,
  targetH,
  cost,
  calls,
}: EdgeArrowProps) {
  const sx = x1 + sourceW;
  const sy = y1 + sourceH / 2;
  const tx = x2;
  const ty = y2 + targetH / 2;
  const mid = (sx + tx) / 2;
  const path = `M ${sx} ${sy} C ${mid} ${sy}, ${mid} ${ty}, ${tx} ${ty}`;

  const angle = Math.atan2(ty - sy, tx - sx);
  const arrowSize = 8;
  const ax = tx - arrowSize * Math.cos(angle);
  const ay = ty - arrowSize * Math.sin(angle);
  const perp = Math.PI / 2;
  const a1x = ax + arrowSize * 0.5 * Math.cos(angle + perp);
  const a1y = ay + arrowSize * 0.5 * Math.sin(angle + perp);
  const a2x = ax + arrowSize * 0.5 * Math.cos(angle - perp);
  const a2y = ay + arrowSize * 0.5 * Math.sin(angle - perp);

  const hasWeight = (cost != null && cost > 0) || (calls != null && calls > 0);
  const strokeWidth =
    hasWeight
      ? Math.min(6, Math.max(1.5, 1.5 + (cost ?? 0) / 1e7 + (calls ?? 0) * 0.05))
      : 1.5;
  const title =
    cost != null && calls != null
      ? `calls: ${calls}, cost: ${cost >= 1e6 ? `${(cost / 1e6).toFixed(2)}M` : cost}`
      : cost != null
        ? `cost: ${cost >= 1e6 ? `${(cost / 1e6).toFixed(2)}M` : cost}`
        : calls != null
          ? `calls: ${calls}`
          : undefined;

  return (
    <g>
      <path
        d={path}
        fill="none"
        stroke="#30363d"
        strokeWidth={strokeWidth}
      >
        {title != null ? <title>{title}</title> : null}
      </path>
      <polygon
        points={`${tx},${ty} ${a1x},${a1y} ${a2x},${a2y}`}
        fill="#30363d"
      />
    </g>
  );
}
