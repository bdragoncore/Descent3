"use client";

import { useEffect, useState } from "react";
import Link from "next/link";

interface GraphEntry {
  filename: string;
  test: string;
}

export default function DashboardPage() {
  const [graphs, setGraphs] = useState<GraphEntry[]>([]);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState<string | null>(null);

  useEffect(() => {
    fetch("/api/graphs")
      .then((r) => r.json())
      .then((data) => {
        setGraphs(Array.isArray(data) ? data : []);
        setError(null);
      })
      .catch((e) => setError(e instanceof Error ? e.message : "Failed to load"))
      .finally(() => setLoading(false));
  }, []);

  if (loading) return <div style={{ padding: 20 }}>Loading...</div>;
  if (error) return <div style={{ padding: 20, color: "#ff7b72" }}>{error}</div>;

  const testSlug = (filename: string) => filename.replace("__graph.json", "");

  return (
    <div style={{ padding: 24, maxWidth: 800 }}>
      <h1 style={{ marginBottom: 16, color: "#58a6ff" }}>Call Graph Viewer</h1>
      <p style={{ color: "#8b949e", marginBottom: 24 }}>
        Select a test to view its call graph (from Valgrind Callgrind).
      </p>
      {graphs.length === 0 ? (
        <p style={{ color: "#8b949e" }}>
          No graph files found. Run render tests with callgrind and set OUTPUT_DIR
          to the render output directory.
        </p>
      ) : (
        <ul style={{ listStyle: "none", padding: 0, margin: 0 }}>
          {graphs.map((g) => (
            <li key={g.filename} style={{ marginBottom: 8 }}>
              <Link
                href={`/graph/${testSlug(g.filename)}`}
                style={{ color: "#79c0ff", textDecoration: "none" }}
              >
                {g.test}
              </Link>
            </li>
          ))}
        </ul>
      )}
    </div>
  );
}
