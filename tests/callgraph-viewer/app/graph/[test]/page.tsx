"use client";

import { useEffect, useState } from "react";
import { useRouter } from "next/navigation";
import { CallGraph } from "@/components/CallGraph";
import type { CallGraph as CallGraphData } from "@/lib/types";

export default function GraphPage({
  params,
}: {
  params: { test: string };
}) {
  const { test } = params;
  const router = useRouter();
  const [graph, setGraph] = useState<CallGraphData | null>(null);
  const [error, setError] = useState<string | null>(null);

  useEffect(() => {
    fetch(`/api/graph/${encodeURIComponent(test)}`)
      .then((r) => {
        if (!r.ok) throw new Error(r.status === 404 ? "Graph not found" : "Failed to load");
        return r.json();
      })
      .then((data) => {
        setGraph(data);
        setError(null);
      })
      .catch((e) => setError(e instanceof Error ? e.message : "Error"));
  }, [test]);

  if (error) {
    return (
      <div style={{ padding: 24 }}>
        <p style={{ color: "#ff7b72" }}>{error}</p>
        <button
          type="button"
          onClick={() => router.push("/")}
          style={{
            marginTop: 12,
            padding: "8px 16px",
            background: "#21262d",
            color: "#c9d1d9",
            border: "1px solid #30363d",
            borderRadius: 6,
            cursor: "pointer",
          }}
        >
          Back to list
        </button>
      </div>
    );
  }

  if (!graph) {
    return <div style={{ padding: 24 }}>Loading...</div>;
  }

  return (
    <div style={{ padding: 16 }}>
      <div style={{ marginBottom: 12, display: "flex", alignItems: "center", gap: 12 }}>
        <button
          type="button"
          onClick={() => router.push("/")}
          style={{
            padding: "6px 12px",
            background: "#21262d",
            color: "#c9d1d9",
            border: "1px solid #30363d",
            borderRadius: 6,
            cursor: "pointer",
          }}
        >
          ← Back
        </button>
        <span style={{ color: "#8b949e" }}>{test.replace(/__/g, ".")}</span>
      </div>
      <CallGraph graph={graph} />
    </div>
  );
}
