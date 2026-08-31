export interface GraphNode {
  fn: string;
  short_fn: string;
  file: string;
  self_cost: number;
  total_cost: number;
  calls_in: number;
  endpoint: boolean;
}

export interface GraphEdge {
  from: string;
  to: string;
  calls: number;
  cost: number;
}

export interface CallGraph {
  root: string;
  nodes: Record<string, GraphNode>;
  edges: GraphEdge[];
}

export interface LayoutNode extends GraphNode {
  id: string;
  x: number;
  y: number;
  depth: number;
}
