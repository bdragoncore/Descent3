/**
 * Type definitions for report generation tools.
 *
 * @module types
 */

/**
 * Represents a single trace event from the C++ tracer.
 */
export interface TraceEvent {
  /** Event type - enter or exit function */
  type: 'enter' | 'exit';
  /** Function name (demangled if available) */
  fn: string;
  /** Function address as hex string */
  addr: string;
  /** Timestamp in nanoseconds */
  ts_ns: number;
}

/**
 * Represents a node in the call tree.
 */
export interface TreeNode {
  /** Function name */
  fn: string;
  /** Function address */
  addr?: string;
  /** Entry timestamp */
  ts: number;
  /** Duration in nanoseconds */
  duration: number;
  /** Child nodes */
  children: TreeNode[];
  /** Number of calls (for aggregated nodes) */
  calls?: number;
  /** Total time including children */
  total_time_ns?: number;
  /** Average time per call */
  avg_time_ns?: number;
  /** Number of child nodes */
  child_count?: number;
  /** Whether this is an endpoint (e.g., ReadPixels) */
  endpoint?: boolean;
}

/**
 * Represents a node in the DAG (Directed Acyclic Graph).
 */
export interface DagNode {
  /** Unique identifier (function name) */
  id: string;
  /** Display label */
  label: string;
}

/**
 * Represents a link/edge in the DAG.
 */
export interface DagLink {
  /** Source function ID */
  source: string;
  /** Target function ID */
  target: string;
  /** Number of calls from source to target */
  count: number;
  /** Total time spent in target when called from source */
  total_ns: number;
}

/**
 * Complete DAG structure.
 */
export interface Dag {
  /** All nodes in the graph */
  nodes: DagNode[];
  /** All links/edges in the graph */
  links: DagLink[];
}

/**
 * Test report data for a single test.
 */
export interface TestReportData {
  /** Test name */
  testName: string;
  /** Test suite/class name */
  testSuite: string;
  /** Executable name */
  executable: string;
  /** Duration in milliseconds */
  durationMs: number;
  /** Current MD5 hash */
  md5Hash: string | null;
  /** Previous baseline MD5 hash */
  previousMd5: string | null;
  /** Test status */
  status: 'PASSED' | 'FAILED';
  /** Output text (if any) */
  outputText: string;
  /** Environment variables */
  environment: Record<string, string>;
  /** List of render functions called */
  renderFunctions: string[];
  /** PNG filename (if test produced image) */
  pngFilename: string | null;
  /** Trace data object (if tracing enabled) */
  traceData: object | null;
  /** List of callgraph HTML files */
  callgraphs: string[];
  /** Whether test passed */
  passed: boolean;
}

/**
 * Statistics for the report summary.
 */
export interface ReportStats {
  /** Total number of tests */
  total: number;
  /** Number of passed tests */
  passed: number;
  /** Number of failed tests */
  failed: number;
  /** Number of MD5 regressions */
  md5Regressions: number;
}

/**
 * Configuration for generating reports.
 */
export interface ReportConfig {
  /** Output directory for HTML files */
  outputDir: string;
  /** Baseline directory for MD5 comparison */
  baselineDir: string | null;
  /** Whether tracing is enabled */
  tracingEnabled: boolean;
  /** Whether to update baseline */
  updateBaseline: boolean;
  /** Viewer base URL for callgraph viewer links */
  viewerBaseUrl: string | null;
}

/**
 * Unit test result (non-render): minimal fields for unit_report.html.
 */
export interface UnitTestReportData {
  /** Executable name */
  executable: string;
  /** Test suite/class name */
  testSuite: string;
  /** Test name */
  testName: string;
  /** Duration in milliseconds */
  durationMs: number;
  /** Test status */
  status: 'PASSED' | 'FAILED';
  /** Whether test passed */
  passed: boolean;
}

/**
 * CLI arguments for the generate script.
 */
export interface CliArgs {
  /** Generation mode */
  mode: 'report' | 'trace' | 'unit-report' | 'combined' | 'all';
  /** Output directory */
  outputDir: string;
  /** Path to results JSON file */
  resultsJson?: string;
  /** List of trace JSON files */
  traceFiles?: string[];
  /** Path to unit test results JSON (for combined mode) */
  unitResultsJson?: string;
  /** Path to render test results JSON (for combined mode) */
  renderResultsJson?: string;
}
