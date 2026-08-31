/**
 * Tree building algorithms for converting flat trace events to nested call trees.
 *
 * Ported from Python generate_html.py
 *
 * @module algorithms/treeBuilder
 */

import type {TraceEvent, TreeNode} from '../types';

/**
 * Builds a hierarchical call tree from flat enter/exit trace events.
 *
 * Algorithm: Stack-based processing
 * - On 'enter': Create node, add to current parent's children, push to stack
 * - On 'exit': Pop from stack, calculate duration
 *
 * @param events - Array of trace events from C++ tracer
 * @returns Root node of the call tree
 */
export function buildTree(events: TraceEvent[]): TreeNode {
  const root: TreeNode = {
    fn: 'root',
    ts: 0,
    duration: 0,
    children: [],
  };

  const stack: TreeNode[] = [root];

  for (const event of events) {
    if (event.type === 'enter') {
      // Create new node
      const node: TreeNode = {
        fn: event.fn,
        addr: event.addr,
        ts: event.ts_ns,
        duration: 0,
        children: [],
      };

      // Add to current parent's children
      const parent = stack[stack.length - 1];
      parent.children.push(node);

      // Push to stack
      stack.push(node);
    } else if (event.type === 'exit') {
      // Pop from stack and calculate duration
      const node = stack.pop();
      if (node) {
        node.duration = event.ts_ns - node.ts;
      }
    }
  }

  return root;
}

/**
 * Prunes std:: nodes from the tree, splicing their children upward.
 *
 * This removes C++ standard library internals while preserving the
 * call flow through them.
 *
 * @param node - Current node to process (modified in place)
 */
export function pruneStdNodes(node: TreeNode): void {
  const kept: TreeNode[] = [];

  for (const child of node.children) {
    // Recursively process child first
    pruneStdNodes(child);

    // If child is a std:: function, promote its children
    if (child.fn.startsWith('std::')) {
      for (const grandchild of child.children) {
        kept.push(grandchild);
      }
    } else {
      kept.push(child);
    }
  }

  node.children = kept;
}

/**
 * Annotates tree nodes with computed statistics.
 *
 * Calculates per node:
 * - calls: Number of times function was called
 * - total_time_ns: Total time including all children
 * - avg_time_ns: Average time per call
 * - child_count: Number of child nodes
 *
 * @param node - Root node to annotate
 */
export function annotateStats(node: TreeNode): void {
  // Calculate total time including children
  let totalTime = node.duration;

  for (const child of node.children) {
    annotateStats(child);
    totalTime += child.total_time_ns || 0;
  }

  // Set statistics
  node.calls = 1;
  node.total_time_ns = totalTime;
  node.avg_time_ns = totalTime;
  node.child_count = node.children.length;

  // Mark endpoint nodes (functions with no children that aren't std::)
  if (node.children.length === 0 && !node.fn.startsWith('std::')) {
    node.endpoint = true;
  }
}

/**
 * Formats duration in nanoseconds to human-readable string.
 *
 * @param ns - Duration in nanoseconds
 * @returns Formatted string (e.g., "1.23ms", "45.67us", "890ns")
 */
export function formatDuration(ns: number): string {
  if (ns < 1000) {
    return `${ns}ns`;
  } else if (ns < 1e6) {
    return `${(ns / 1000).toFixed(2)}us`;
  } else if (ns < 1e9) {
    return `${(ns / 1e6).toFixed(2)}ms`;
  } else {
    return `${(ns / 1e9).toFixed(3)}s`;
  }
}

/**
 * Counts total nodes in the tree.
 *
 * @param node - Root node
 * @returns Total node count
 */
export function countNodes(node: TreeNode): number {
  let count = 1;
  for (const child of node.children) {
    count += countNodes(child);
  }
  return count;
}

/**
 * Finds the path from test method to ReadPixels or other end marker.
 *
 * @param node - Root of tree to search
 * @param testMethod - Name of test method to start from
 * @returns Linear path of nodes from test to end, or null if not found
 */
export function findPathToReadPixels(
  node: TreeNode,
  testMethod: string,
): TreeNode[] | null {
  const END_MARKERS = ['ReadPixels', 'glFinish', 'glFlush', 'eglSwapBuffers'];

  // Find the test method node
  const findTestNode = (n: TreeNode): TreeNode | null => {
    if (n.fn.includes(testMethod)) {
      return n;
    }
    for (const child of n.children) {
      const found = findTestNode(child);
      if (found) return found;
    }
    return null;
  };

  const testNode = findTestNode(node);
  if (!testNode) return null;

  // Build path from test to end marker
  const path: TreeNode[] = [testNode];
  let current = testNode;

  while (current.children.length > 0) {
    // Find child that leads to end marker
    let nextChild: TreeNode | null = null;

    for (const child of current.children) {
      if (END_MARKERS.some((marker) => child.fn.includes(marker))) {
        nextChild = child;
        break;
      }
    }

    // If no end marker found, take first child
    if (!nextChild && current.children.length > 0) {
      nextChild = current.children[0];
    }

    if (!nextChild) break;

    path.push(nextChild);
    current = nextChild;

    // Stop if we hit an end marker
    if (END_MARKERS.some((marker) => current.fn.includes(marker))) {
      break;
    }
  }

  return path.length > 1 ? path : null;
}

/**
 * Converts a linear path to a slim tree structure.
 *
 * @param path - Linear path of nodes
 * @returns Root of slim tree
 */
export function pathToSlimTree(path: TreeNode[]): TreeNode {
  if (path.length === 0) {
    return {fn: 'root', ts: 0, duration: 0, children: []};
  }

  // Build chain from end to start
  let current: TreeNode = {
    ...path[path.length - 1],
    children: [],
  };

  for (let i = path.length - 2; i >= 0; i--) {
    current = {
      ...path[i],
      children: [current],
    };
  }

  return current;
}

/**
 * Merges repeated sibling calls into single nodes with call counts.
 *
 * @param node - Root node to process (modified in place)
 */
export function mergeRepeatedCalls(node: TreeNode): void {
  const merged: TreeNode[] = [];
  const fnMap = new Map<string, TreeNode>();

  for (const child of node.children) {
    if (fnMap.has(child.fn)) {
      // Merge with existing
      const existing = fnMap.get(child.fn)!;
      existing.calls = (existing.calls || 1) + 1;
      existing.duration += child.duration;
      existing.total_time_ns = (existing.total_time_ns || 0) + (child.total_time_ns || 0);

      // Merge children
      for (const grandchild of child.children) {
        existing.children.push(grandchild);
      }
    } else {
      // New unique function
      fnMap.set(child.fn, child);
      merged.push(child);
    }

    // Recursively process child
    mergeRepeatedCalls(child);
  }

  node.children = merged;

  // Recalculate stats after merging
  if (node.calls && node.calls > 1) {
    node.avg_time_ns = (node.total_time_ns || 0) / node.calls;
  }
}
