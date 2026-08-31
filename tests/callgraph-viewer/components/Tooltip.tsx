"use client";

interface TooltipProps {
  x: number;
  y: number;
  title: string;
  content?: string;
  visible: boolean;
}

export function Tooltip({ x, y, title, content, visible }: TooltipProps) {
  if (!visible) return null;
  return (
    <div
      style={{
        position: "fixed",
        left: x + 12,
        top: y + 12,
        background: "#161b22",
        border: "1px solid #30363d",
        borderRadius: 6,
        padding: "8px 12px",
        maxWidth: 400,
        fontSize: 12,
        color: "#c9d1d9",
        pointerEvents: "none",
        zIndex: 1000,
        boxShadow: "0 4px 12px rgba(0,0,0,0.3)",
      }}
    >
      <div style={{ fontWeight: 600, marginBottom: 4, color: "#79c0ff" }}>
        {title}
      </div>
      {content && (
        <div style={{ color: "#8b949e", wordBreak: "break-all" }}>{content}</div>
      )}
    </div>
  );
}
