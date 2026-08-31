/**
 * DAG (Directed Acyclic Graph) building algorithms for call graph visualization.
 *
 * Ported from Python generate_html.py
 *
 * @module algorithms/dagBuilder
 */

import type {TraceEvent, Dag, DagNode, DagLink} from '../types';

/**
 * Builds a DAG from flat trace events.
 *
 * Tracks caller -> callee relationships with call counts and total duration.
 * This creates a graph suitable for force-directed visualization.
 *
 * Algorithm:
 * - Track call stack
 * - On each 'enter', record caller->callee link and increment count
 * - On each 'exit', calculate duration for the link
 *
 * @param events - Array of trace events
 * @returns DAG with nodes and links
 */
export function buildDag(events: TraceEvent[]): Dag {
  const nodesMap = new Map<string, DagNode>();
  const linksMap = new Map<string, DagLink>();

  // Stack tracks current call chain: { fn, ts }
  const stack: Array<{fn: string; ts: number}> = [];
  const ROOT_ID = '<root>';

  // Helper to get or create node
  const getOrCreateNode = (fn: string): DagNode => {
    if (!nodesMap.has(fn)) {
      nodesMap.set(fn, {
        id: fn,
        label: fn,
      });
    }
    return nodesMap.get(fn)!;
  };

  // Helper to get link key
  const getLinkKey = (source: string, target: string): string => {
    return `${source}→${target}`;
  };

  // Helper to get or create link
  const getOrCreateLink = (source: string, target: string): DagLink => {
    const key = getLinkKey(source, target);
    if (!linksMap.has(key)) {
      linksMap.set(key, {
        source,
        target,
        count: 0,
        total_ns: 0,
      });
    }
    return linksMap.get(key)!;
  };

  for (const event of events) {
    if (event.type === 'enter') {
      // Get caller (parent in stack, or root if stack is empty)
      const caller = stack.length > 0 ? stack[stack.length - 1].fn : ROOT_ID;

      // Ensure both nodes exist
      getOrCreateNode(event.fn);
      if (caller !== ROOT_ID) {
        getOrCreateNode(caller);
      }

      // Increment link count
      const link = getOrCreateLink(caller, event.fn);
      link.count++;

      // Push to stack
      stack.push({
        fn: event.fn,
        ts: event.ts_ns,
      });
    } else if (event.type === 'exit') {
      // Pop from stack
      const exited = stack.pop();
      if (!exited) continue;

      // Calculate duration for the link
      const duration = event.ts_ns - exited.ts;

      // Find the link and add duration
      // The link is from caller (now top of stack) to exited function
      const caller = stack.length > 0 ? stack[stack.length - 1].fn : ROOT_ID;
      const key = getLinkKey(caller, exited.fn);

      if (linksMap.has(key)) {
        const link = linksMap.get(key)!;
        link.total_ns += duration;
      }
    }
  }

  return {
    nodes: Array.from(nodesMap.values()),
    links: Array.from(linksMap.values()),
  };
}

/**
 * Aggregates multiple DAGs into a single unified graph.
 *
 * Used to build the top-level call graph from multiple trace files.
 *
 * @param dags - Array of DAGs to aggregate
 * @returns Unified DAG
 */
export function aggregateDags(dags: Dag[]): Dag {
  const nodesMap = new Map<string, DagNode>();
  const linksMap = new Map<string, DagLink>();

  for (const dag of dags) {
    // Merge nodes
    for (const node of dag.nodes) {
      if (!nodesMap.has(node.id)) {
        nodesMap.set(node.id, {...node});
      }
    }

    // Merge links (sum counts and durations)
    for (const link of dag.links) {
      const key = `${link.source}→${link.target}`;

      if (!linksMap.has(key)) {
        linksMap.set(key, {
          source: link.source,
          target: link.target,
          count: 0,
          total_ns: 0,
        });
      }

      const aggregated = linksMap.get(key)!;
      aggregated.count += link.count;
      aggregated.total_ns += link.total_ns;
    }
  }

  return {
    nodes: Array.from(nodesMap.values()),
    links: Array.from(linksMap.values()),
  };
}

/**
 * Limits DAG to top N nodes by total call count.
 *
 * Useful for keeping large graphs manageable.
 *
 * @param dag - Input DAG
 * @param topN - Number of top nodes to keep
 * @returns Truncated DAG and note about truncation
 */
export function truncateDag(dag: Dag, topN: number): {dag: Dag; note: string | null} {
  // Calculate total call count per node (both sources and targets)
  const countById = new Map<string, number>();

  // Initialize counts for all nodes in the DAG
  for (const node of dag.nodes) {
    countById.set(node.id, 0);
  }

  for (const link of dag.links) {
    countById.set(link.source, (countById.get(link.source) || 0) + link.count);
    countById.set(link.target, (countById.get(link.target) || 0) + link.count);
  }

  // Sort by count and take top N (only from original nodes)
  const nodeIds = new Set(dag.nodes.map(n => n.id));
  const sortedIds = Array.from(countById.entries())
    .filter(([id]) => nodeIds.has(id))
    .sort((a, b) => b[1] - a[1])
    .map(([id]) => id);

  const keep = new Set(sortedIds.slice(0, topN));

  // Filter nodes and links
  const nodesTop = dag.nodes.filter((n) => keep.has(n.id));
  const linksTop = dag.links.filter(
    (l) => keep.has(l.source) && keep.has(l.target),
  );

  const result: Dag = {
    nodes: nodesTop,
    links: linksTop,
  };

  let note: string | null = null;
  if (dag.nodes.length > topN) {
    note = `Showing top ${topN} of ${dag.nodes.length} nodes.`;
  }

  return {dag: result, note};
}

/**
 * Calculates maximum total_ns for scaling.
 *
 * @param dag - DAG with links
 * @returns Maximum total_ns value
 */
export function getMaxDuration(dag: Dag): number {
  if (dag.links.length === 0) return 1;

  return Math.max(...dag.links.map((l) => l.total_ns || 0), 1);
}

/**
 * Formats link information for tooltips.
 *
 * @param link - DAG link
 * @returns Formatted string for display
 */
export function formatLinkTooltip(link: DagLink): string {
  const count = link.count;
  const duration = link.total_ns;

  let durationStr: string;
  if (duration < 1e3) {
    durationStr = `${duration}ns`;
  } else if (duration < 1e6) {
    durationStr = `${(duration / 1e3).toFixed(2)}us`;
  } else {
    durationStr = `${(duration / 1e6).toFixed(2)}ms`;
  }

  if (count > 0 && duration > 0) {
    return `calls: ${count}, time: ${durationStr}`;
  } else if (count > 0) {
    return `calls: ${count}`;
  }
  return '';
}

/**
 * Calculates stroke width for link based on duration.
 *
 * @param totalNs - Duration of this link
 * @param maxNs - Maximum duration in the graph
 * @returns Stroke width (minimum 1)
 */
export function calculateStrokeWidth(totalNs: number, maxNs: number): number {
  if (totalNs === 0 || maxNs === 0) return 1;
  return Math.max(1, 1 + 2 * (totalNs / maxNs));
}
