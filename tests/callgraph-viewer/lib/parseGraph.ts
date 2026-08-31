import type { CallGraph } from "./types";

export async function fetchGraph(testSlug: string): Promise<CallGraph> {
  const res = await fetch(`/api/graph/${encodeURIComponent(testSlug)}`);
  if (!res.ok) {
    const err = await res.json().catch(() => ({}));
    throw new Error((err as { error?: string }).error ?? `Failed to load graph: ${res.status}`);
  }
  const data = await res.json();
  return validateGraph(data);
}

function validateGraph(data: unknown): CallGraph {
  if (!data || typeof data !== "object") throw new Error("Invalid graph: not an object");
  const o = data as Record<string, unknown>;
  if (typeof o.root !== "string") throw new Error("Invalid graph: missing or invalid root");
  if (!o.nodes || typeof o.nodes !== "object") throw new Error("Invalid graph: missing or invalid nodes");
  if (!Array.isArray(o.edges)) throw new Error("Invalid graph: missing or invalid edges");
  return {
    root: o.root,
    nodes: o.nodes as Record<string, import("./types").GraphNode>,
    edges: o.edges as import("./types").GraphEdge[],
  };
}
