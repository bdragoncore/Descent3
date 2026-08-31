"use client";

import { D3_MODULES, type D3Module } from "@/lib/filter";

interface ModuleLegendProps {
  enabledModules: Set<string>;
  onToggle: (moduleName: string) => void;
}

export function ModuleLegend({ enabledModules, onToggle }: ModuleLegendProps) {
  const allEnabled = enabledModules.size === 0;

  return (
    <div
      style={{
        background: "#0d1117",
        border: "1px solid #30363d",
        borderRadius: 8,
        padding: 12,
        marginBottom: 12,
        maxWidth: 400,
      }}
    >
      <div style={{ color: "#8b949e", fontSize: 13, marginBottom: 8 }}>
        D3 Pipeline Modules
      </div>
      <div style={{ display: "flex", flexWrap: "wrap", gap: 6 }}>
        {D3_MODULES.map((mod) => {
          const isOn = allEnabled || enabledModules.has(mod.name);
          return (
            <button
              key={mod.name}
              onClick={() => onToggle(mod.name)}
              title={mod.description}
              style={{
                background: isOn ? `${mod.color}22` : "transparent",
                color: isOn ? mod.color : "#484f58",
                border: `1px solid ${isOn ? mod.color : "#30363d"}`,
                borderRadius: 4,
                padding: "2px 8px",
                fontSize: 12,
                cursor: "pointer",
                fontFamily: "monospace",
                opacity: isOn ? 1 : 0.5,
              }}
            >
              {mod.prefix}
            </button>
          );
        })}
      </div>
      <div style={{ marginTop: 8, display: "flex", gap: 8, fontSize: 12 }}>
        <label style={{ color: "#8b949e", cursor: "pointer" }}>
          <input
            type="checkbox"
            checked={allEnabled}
            onChange={() => {
              if (!allEnabled) onToggle("__all__");
              else {
                // To disable all, we'd need to clear — handled by parent
              }
            }}
            style={{ marginRight: 4 }}
          />
          Show all
        </label>
      </div>
    </div>
  );
}
