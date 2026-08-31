"use client";

interface ToolbarProps {
  onResetLayout: () => void;
  onFitView: () => void;
}

export function Toolbar({ onResetLayout, onFitView }: ToolbarProps) {
  const btnStyle: React.CSSProperties = {
    padding: "6px 12px",
    marginRight: 8,
    background: "#21262d",
    color: "#c9d1d9",
    border: "1px solid #30363d",
    borderRadius: 6,
    cursor: "pointer",
    fontSize: 12,
  };

  return (
    <div style={{ marginBottom: 12, display: "flex", alignItems: "center" }}>
      <button type="button" style={btnStyle} onClick={onResetLayout}>
        Reset Layout
      </button>
      <button type="button" style={btnStyle} onClick={onFitView}>
        Fit View
      </button>
    </div>
  );
}
