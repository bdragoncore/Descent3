/**
 * Tests for DAG building algorithms.
 *
 * @module __tests__/dagBuilder.test
 */

import {describe, it, expect} from 'vitest';
import {
  buildDag,
  aggregateDags,
  truncateDag,
  getMaxDuration,
  formatLinkTooltip,
  calculateStrokeWidth,
} from '../src/algorithms/dagBuilder';
import type {TraceEvent, Dag} from '../src/types';

describe('buildDag', () => {
  it('should build a simple DAG from enter/exit events', () => {
    const events: TraceEvent[] = [
      {type: 'enter', fn: 'main', addr: '0x1', ts_ns: 1000},
      {type: 'enter', fn: 'foo', addr: '0x2', ts_ns: 2000},
      {type: 'exit', fn: 'foo', addr: '0x2', ts_ns: 5000},
      {type: 'exit', fn: 'main', addr: '0x1', ts_ns: 10000},
    ];

    const dag = buildDag(events);

    expect(dag.nodes).toHaveLength(2);
    expect(dag.links).toHaveLength(2); // root->main, main->foo

    const rootToMain = dag.links.find((l) => l.source === '<root>' && l.target === 'main');
    expect(rootToMain).toBeDefined();
    expect(rootToMain?.count).toBe(1);
  });

  it('should aggregate multiple calls', () => {
    const events: TraceEvent[] = [
      {type: 'enter', fn: 'main', addr: '0x1', ts_ns: 1000},
      {type: 'enter', fn: 'foo', addr: '0x2', ts_ns: 2000},
      {type: 'exit', fn: 'foo', addr: '0x2', ts_ns: 3000},
      {type: 'enter', fn: 'foo', addr: '0x2', ts_ns: 4000},
      {type: 'exit', fn: 'foo', addr: '0x2', ts_ns: 5000},
      {type: 'exit', fn: 'main', addr: '0x1', ts_ns: 10000},
    ];

    const dag = buildDag(events);

    const mainToFoo = dag.links.find((l) => l.source === 'main' && l.target === 'foo');
    expect(mainToFoo).toBeDefined();
    expect(mainToFoo?.count).toBe(2);
  });

  it('should calculate total_ns correctly', () => {
    const events: TraceEvent[] = [
      {type: 'enter', fn: 'main', addr: '0x1', ts_ns: 1000},
      {type: 'enter', fn: 'foo', addr: '0x2', ts_ns: 2000},
      {type: 'exit', fn: 'foo', addr: '0x2', ts_ns: 5000},
      {type: 'exit', fn: 'main', addr: '0x1', ts_ns: 10000},
    ];

    const dag = buildDag(events);

    const mainToFoo = dag.links.find((l) => l.source === 'main' && l.target === 'foo');
    expect(mainToFoo?.total_ns).toBe(3000); // 5000 - 2000
  });

  it('should handle empty events', () => {
    const dag = buildDag([]);

    expect(dag.nodes).toHaveLength(0);
    expect(dag.links).toHaveLength(0);
  });

  it('should handle complex call patterns', () => {
    const events: TraceEvent[] = [
      // main calls A, A calls B and C
      {type: 'enter', fn: 'main', addr: '0x1', ts_ns: 1000},
      {type: 'enter', fn: 'A', addr: '0x2', ts_ns: 2000},
      {type: 'enter', fn: 'B', addr: '0x3', ts_ns: 3000},
      {type: 'exit', fn: 'B', addr: '0x3', ts_ns: 4000},
      {type: 'enter', fn: 'C', addr: '0x4', ts_ns: 5000},
      {type: 'exit', fn: 'C', addr: '0x4', ts_ns: 6000},
      {type: 'exit', fn: 'A', addr: '0x2', ts_ns: 7000},
      {type: 'exit', fn: 'main', addr: '0x1', ts_ns: 8000},
    ];

    const dag = buildDag(events);

    expect(dag.nodes).toHaveLength(4); // main, A, B, C
    expect(dag.links).toHaveLength(4); // root->main, main->A, A->B, A->C
  });
});

describe('aggregateDags', () => {
  it('should merge nodes from multiple DAGs', () => {
    const dag1: Dag = {
      nodes: [{id: 'A', label: 'A'}],
      links: [{source: '<root>', target: 'A', count: 1, total_ns: 1000}],
    };

    const dag2: Dag = {
      nodes: [{id: 'B', label: 'B'}],
      links: [{source: '<root>', target: 'B', count: 1, total_ns: 2000}],
    };

    const aggregated = aggregateDags([dag1, dag2]);

    expect(aggregated.nodes).toHaveLength(2);
    expect(aggregated.links).toHaveLength(2);
  });

  it('should sum counts and durations for same links', () => {
    const dag1: Dag = {
      nodes: [{id: 'A', label: 'A'}],
      links: [{source: '<root>', target: 'A', count: 2, total_ns: 3000}],
    };

    const dag2: Dag = {
      nodes: [{id: 'A', label: 'A'}],
      links: [{source: '<root>', target: 'A', count: 3, total_ns: 5000}],
    };

    const aggregated = aggregateDags([dag1, dag2]);

    const link = aggregated.links[0];
    expect(link.count).toBe(5); // 2 + 3
    expect(link.total_ns).toBe(8000); // 3000 + 5000
  });

  it('should handle empty array', () => {
    const aggregated = aggregateDags([]);

    expect(aggregated.nodes).toHaveLength(0);
    expect(aggregated.links).toHaveLength(0);
  });
});

describe('truncateDag', () => {
  it('should keep top N nodes by call count', () => {
    const dag: Dag = {
      nodes: [
        {id: 'A', label: 'A'},
        {id: 'B', label: 'B'},
        {id: 'C', label: 'C'},
        {id: 'D', label: 'D'},
      ],
      links: [
        {source: '<root>', target: 'A', count: 10, total_ns: 1000},
        {source: '<root>', target: 'B', count: 5, total_ns: 500},
        {source: '<root>', target: 'C', count: 3, total_ns: 300},
        {source: '<root>', target: 'D', count: 1, total_ns: 100},
      ],
    };

    const {dag: truncated, note} = truncateDag(dag, 2);

    expect(truncated.nodes).toHaveLength(2);
    expect(note).toBe('Showing top 2 of 4 nodes.');
  });

  it('should return null note when not truncated', () => {
    const dag: Dag = {
      nodes: [{id: 'A', label: 'A'}],
      links: [{source: '<root>', target: 'A', count: 1, total_ns: 1000}],
    };

    const {dag: truncated, note} = truncateDag(dag, 5);

    expect(truncated.nodes).toHaveLength(1);
    expect(note).toBeNull();
  });

  it('should filter links that reference removed nodes', () => {
    const dag: Dag = {
      nodes: [
        {id: 'A', label: 'A'},
        {id: 'B', label: 'B'},
        {id: 'C', label: 'C'},
      ],
      links: [
        {source: '<root>', target: 'A', count: 10, total_ns: 1000},
        {source: 'A', target: 'B', count: 5, total_ns: 500},
        {source: 'B', target: 'C', count: 3, total_ns: 300},
      ],
    };

    const {dag: truncated} = truncateDag(dag, 2);

    // Only links where both source and target are kept
    expect(truncated.links.length).toBeLessThanOrEqual(2);
  });
});

describe('getMaxDuration', () => {
  it('should return max duration from links', () => {
    const dag: Dag = {
      nodes: [],
      links: [
        {source: 'A', target: 'B', count: 1, total_ns: 1000},
        {source: 'B', target: 'C', count: 1, total_ns: 5000},
        {source: 'C', target: 'D', count: 1, total_ns: 3000},
      ],
    };

    expect(getMaxDuration(dag)).toBe(5000);
  });

  it('should return 1 for empty links', () => {
    const dag: Dag = {
      nodes: [],
      links: [],
    };

    expect(getMaxDuration(dag)).toBe(1);
  });
});

describe('formatLinkTooltip', () => {
  it('should format with count and duration', () => {
    const link = {source: 'A', target: 'B', count: 5, total_ns: 1500000};

    expect(formatLinkTooltip(link)).toBe('calls: 5, time: 1.50ms');
  });

  it('should format with count only when duration is 0', () => {
    const link = {source: 'A', target: 'B', count: 3, total_ns: 0};

    expect(formatLinkTooltip(link)).toBe('calls: 3');
  });

  it('should handle nanoseconds', () => {
    const link = {source: 'A', target: 'B', count: 2, total_ns: 500};

    expect(formatLinkTooltip(link)).toBe('calls: 2, time: 500ns');
  });

  it('should handle microseconds', () => {
    const link = {source: 'A', target: 'B', count: 1, total_ns: 50000};

    expect(formatLinkTooltip(link)).toBe('calls: 1, time: 50.00us');
  });
});

describe('calculateStrokeWidth', () => {
  it('should calculate correct width', () => {
    expect(calculateStrokeWidth(500, 1000)).toBe(2);
    expect(calculateStrokeWidth(1000, 1000)).toBe(3);
  });

  it('should return minimum of 1', () => {
    expect(calculateStrokeWidth(0, 1000)).toBe(1);
    expect(calculateStrokeWidth(100, 0)).toBe(1);
  });

  it('should handle zero max', () => {
    expect(calculateStrokeWidth(500, 0)).toBe(1);
  });
});
