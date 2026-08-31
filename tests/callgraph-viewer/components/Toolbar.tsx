"use client";

import type { FilterOptions, FilterReport } from "@/lib/filter";

interface ToolbarProps {
  onResetLayout: () => void;
  onFitView: () => void;
  filterOptions: FilterOptions;
  onFilterChange: (updates: Partial<FilterOptions>) => void;
  report?: FilterReport | null;
}

export function Toolbar({
  onResetLayout,
  onFitView,
  filterOptions,
  onFilterChange,
  report,
}: ToolbarProps) {
  const btn: React.CSSProperties = {
    padding: "6px 12px",
    marginRight: 8,
    background: "#21262d",
    color: "#c9d1d9",
    border: "1px solid #30363d",
    borderRadius: 6,
    cursor: "pointer",
    fontSize: 12,
  };
  const label: React.CSSProperties = {
    color: "#8b949e",
    fontSize: 12,
    marginRight: 4,
    marginLeft: 8,
  };
  const input: React.CSSProperties = {
    background: "#0d1117",
    color: "#c9d1d9",
    border: "1px solid #30363d",
    borderRadius: 4,
    padding: "3px 6px",
    fontSize: 12,
    width: 100,
    fontFamily: "monospace",
  };
  const slider: React.CSSProperties = {
    margin: "0 4px",
    verticalAlign: "middle",
    width: 80,
  };
  const badge: React.CSSProperties = {
    fontSize: 11,
    color: "#8b949e",
    marginLeft: 8,
    padding: "2px 6px",
    background: "#161b22",
    borderRadius: 4,
    border: "1px solid #30363d",
  };

  return (
    <div
      style={{
        marginBottom: 8,
        padding: 8,
        background: "#0d1117",
        border: "1px solid #30363d",
        borderRadius: 8,
      }}
    >
      {/* Row 1: View controls */}
      <div style={{ display: "flex", alignItems: "center", marginBottom: 6 }}>
        <button type="button" style={btn} onClick={onResetLayout}>
          Reset Layout
        </button>
        <button type="button" style={btn} onClick={onFitView}>
          Fit View
        </button>
        {report && (
          <span style={badge}>
            {report.keptNodes} nodes ({report.totalNodes - report.keptNodes} filtered)
          </span>
        )}
      </div>

      {/* Row 2: Filter toggles */}
      <div style={{ display: "flex", alignItems: "center", flexWrap: "wrap", gap: 4 }}>
        <label style={label}>
          <input
            type="checkbox"
            checked={filterOptions.hideStdlib}
            onChange={(e) => onFilterChange({ hideStdlib: e.target.checked })}
          />
          Hide stdlib
        </label>
        <label style={label}>
          <input
            type="checkbox"
            checked={filterOptions.hideMesaInternal}
            onChange={(e) => onFilterChange({ hideMesaInternal: e.target.checked })}
          />
          Hide Mesa
        </label>
        <label style={label}>
          <input
            type="checkbox"
            checked={filterOptions.groupByModule}
            onChange={(e) => onFilterChange({ groupByModule: e.target.checked })}
          />
          Group modules
        </label>

        <span style={label}>Root:</span>
        <input
          style={{ ...input, width: 90 }}
          value={filterOptions.rootFn}
          onChange={(e) => onFilterChange({ rootFn: e.target.value })}
        />
        <span style={label}>Leaf:</span>
        <input
          style={{ ...input, width: 90 }}
          value={filterOptions.leafFn}
          onChange={(e) => onFilterChange({ leafFn: e.target.value })}
        />

        <span style={label}>Depth:</span>
        <input
          type="range"
          min={2}
          max={20}
          value={filterOptions.maxDepth}
          onChange={(e) => onFilterChange({ maxDepth: parseInt(e.target.value) })}
          style={slider}
        />
        <span style={badge}>{filterOptions.maxDepth}</span>

        <span style={label}>Cost%:</span>
        <input
          type="range"
          min={0}
          max={50}
          value={Math.round(filterOptions.minCostPct * 10000)}
          onChange={(e) => onFilterChange({ minCostPct: parseInt(e.target.value) / 10000 })}
          style={slider}
        />
        <span style={badge}>{(filterOptions.minCostPct * 100).toFixed(2)}%</span>
      </div>
    </div>
  );
}
