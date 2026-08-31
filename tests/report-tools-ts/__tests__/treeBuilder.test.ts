/**
 * Tests for tree building algorithms.
 *
 * @module __tests__/treeBuilder.test
 */

import {describe, it, expect} from 'vitest';
import {
  buildTree,
  pruneStdNodes,
  annotateStats,
  formatDuration,
  countNodes,
} from '../src/algorithms/treeBuilder';
import type {TraceEvent, TreeNode} from '../src/types';

describe('buildTree', () => {
  it('should build a simple tree from enter/exit events', () => {
    const events: TraceEvent[] = [
      {type: 'enter', fn: 'main', addr: '0x1', ts_ns: 1000},
      {type: 'enter', fn: 'foo', addr: '0x2', ts_ns: 2000},
      {type: 'exit', fn: 'foo', addr: '0x2', ts_ns: 5000},
      {type: 'exit', fn: 'main', addr: '0x1', ts_ns: 10000},
    ];

    const tree = buildTree(events);

    expect(tree.fn).toBe('root');
    expect(tree.children).toHaveLength(1);
    expect(tree.children[0].fn).toBe('main');
    expect(tree.children[0].children).toHaveLength(1);
    expect(tree.children[0].children[0].fn).toBe('foo');
  });

  it('should calculate correct durations', () => {
    const events: TraceEvent[] = [
      {type: 'enter', fn: 'main', addr: '0x1', ts_ns: 1000},
      {type: 'exit', fn: 'main', addr: '0x1', ts_ns: 10000},
    ];

    const tree = buildTree(events);
    const mainNode = tree.children[0];

    expect(mainNode.duration).toBe(9000);
  });

  it('should handle nested function calls', () => {
    const events: TraceEvent[] = [
      {type: 'enter', fn: 'A', addr: '0x1', ts_ns: 1000},
      {type: 'enter', fn: 'B', addr: '0x2', ts_ns: 2000},
      {type: 'enter', fn: 'C', addr: '0x3', ts_ns: 3000},
      {type: 'exit', fn: 'C', addr: '0x3', ts_ns: 4000},
      {type: 'exit', fn: 'B', addr: '0x2', ts_ns: 5000},
      {type: 'exit', fn: 'A', addr: '0x1', ts_ns: 6000},
    ];

    const tree = buildTree(events);

    expect(tree.children[0].fn).toBe('A');
    expect(tree.children[0].children[0].fn).toBe('B');
    expect(tree.children[0].children[0].children[0].fn).toBe('C');
  });

  it('should handle empty events', () => {
    const tree = buildTree([]);
    expect(tree.fn).toBe('root');
    expect(tree.children).toHaveLength(0);
  });
});

describe('pruneStdNodes', () => {
  it('should remove std:: nodes and promote children', () => {
    const tree: TreeNode = {
      fn: 'root',
      ts: 0,
      duration: 0,
      children: [
        {
          fn: 'main',
          ts: 1000,
          duration: 9000,
          children: [
            {
              fn: 'std::vector::push_back',
              ts: 2000,
              duration: 1000,
              children: [
                {
                  fn: 'actual_function',
                  ts: 2500,
                  duration: 500,
                  children: [],
                },
              ],
            },
          ],
        },
      ],
    };

    pruneStdNodes(tree);

    expect(tree.children[0].children).toHaveLength(1);
    expect(tree.children[0].children[0].fn).toBe('actual_function');
  });

  it('should handle multiple std:: nodes', () => {
    const tree: TreeNode = {
      fn: 'root',
      ts: 0,
      duration: 0,
      children: [
        {
          fn: 'main',
          ts: 1000,
          duration: 1000,
          children: [
            {
              fn: 'std::string',
              ts: 2000,
              duration: 500,
              children: [
                {
                  fn: 'std::allocator',
                  ts: 2100,
                  duration: 400,
                  children: [
                    {
                      fn: 'malloc',
                      ts: 2200,
                      duration: 300,
                      children: [],
                    },
                  ],
                },
              ],
            },
          ],
        },
      ],
    };

    pruneStdNodes(tree);

    // Both std:: nodes should be removed, leaving malloc under main
    expect(tree.children[0].children).toHaveLength(1);
    expect(tree.children[0].children[0].fn).toBe('malloc');
  });

  it('should not modify non-std nodes', () => {
    const tree: TreeNode = {
      fn: 'root',
      ts: 0,
      duration: 0,
      children: [
        {
          fn: 'main',
          ts: 1000,
          duration: 1000,
          children: [
            {
              fn: 'game_function',
              ts: 2000,
              duration: 500,
              children: [],
            },
          ],
        },
      ],
    };

    pruneStdNodes(tree);

    expect(tree.children[0].children).toHaveLength(1);
    expect(tree.children[0].children[0].fn).toBe('game_function');
  });
});

describe('annotateStats', () => {
  it('should calculate total_time_ns correctly', () => {
    const tree: TreeNode = {
      fn: 'root',
      ts: 0,
      duration: 0,
      children: [
        {
          fn: 'main',
          ts: 1000,
          duration: 1000,
          children: [
            {
              fn: 'child1',
              ts: 2000,
              duration: 500,
              children: [],
            },
            {
              fn: 'child2',
              ts: 3000,
              duration: 300,
              children: [],
            },
          ],
        },
      ],
    };

    annotateStats(tree);

    const mainNode = tree.children[0];
    expect(mainNode.total_time_ns).toBe(1800); // 1000 + 500 + 300
    expect(mainNode.child_count).toBe(2);
  });

  it('should set endpoint flag correctly', () => {
    const tree: TreeNode = {
      fn: 'root',
      ts: 0,
      duration: 0,
      children: [
        {
          fn: 'main',
          ts: 1000,
          duration: 1000,
          children: [
            {
              fn: 'leaf',
              ts: 2000,
              duration: 500,
              children: [],
            },
          ],
        },
      ],
    };

    annotateStats(tree);

    expect(tree.children[0].children[0].endpoint).toBe(true);
    expect(tree.children[0].endpoint).toBeUndefined();
  });

  it('should calculate avg_time_ns', () => {
    const tree: TreeNode = {
      fn: 'root',
      ts: 0,
      duration: 0,
      children: [
        {
          fn: 'main',
          ts: 1000,
          duration: 1000,
          children: [],
        },
      ],
    };

    annotateStats(tree);

    expect(tree.children[0].avg_time_ns).toBe(1000);
  });
});

describe('formatDuration', () => {
  it('should format nanoseconds', () => {
    expect(formatDuration(500)).toBe('500ns');
  });

  it('should format microseconds', () => {
    expect(formatDuration(1500)).toBe('1.50us');
    expect(formatDuration(50000)).toBe('50.00us');
  });

  it('should format milliseconds', () => {
    expect(formatDuration(1500000)).toBe('1.50ms');
    expect(formatDuration(50000000)).toBe('50.00ms');
  });

  it('should format seconds', () => {
    expect(formatDuration(1500000000)).toBe('1.500s');
    expect(formatDuration(5000000000)).toBe('5.000s');
  });
});

describe('countNodes', () => {
  it('should count nodes correctly', () => {
    const tree: TreeNode = {
      fn: 'root',
      ts: 0,
      duration: 0,
      children: [
        {
          fn: 'A',
          ts: 1000,
          duration: 1000,
          children: [
            {
              fn: 'B',
              ts: 2000,
              duration: 500,
              children: [],
            },
          ],
        },
      ],
    };

    expect(countNodes(tree)).toBe(3); // root + A + B
  });

  it('should return 1 for empty tree', () => {
    const tree: TreeNode = {
      fn: 'root',
      ts: 0,
      duration: 0,
      children: [],
    };

    expect(countNodes(tree)).toBe(1);
  });
});
