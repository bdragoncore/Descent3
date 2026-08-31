/**
 * callgraph-viewer — graph filtering pipeline.
 *
 * Transforms a raw 3300-node call graph into a high-level rendering flow
 * by filtering stdlib/Mesa noise, extracting the root→leaf path, grouping
 * by D3 subsystem, and pruning low-cost nodes.
 *
 * Pipeline order:
 *   1. Name-based stdlib + Mesa filtering
 *   2. Bidirectional reachability (main ∩ glFlush ancestors)
 *   3. Cost threshold pruning
 *   4. Module grouping (g3_*, rend_*, gl*, …)
 *   5. Depth limiting
 */

import type { CallGraph, GraphNode, GraphEdge } from "./types";

// ── Prefix lists ─────────────────────────────────────────────────────────

/** C++ standard library and runtime functions to hide. */
const STDLIB_PREFIXES = [
  "std::",
  "__gnu_cxx::",
  "__cxxabiv1::",
  "boost::",
  // glibc / allocation
  "malloc",
  "free",
  "calloc",
  "realloc",
  "valloc",
  "mmap",
  "munmap",
  "brk",
  "sbrk",
  "operator new",
  "operator delete",
  // string / memory builtins
  "memcpy",
  "memset",
  "memmove",
  "memcmp",
  "strlen",
  "strcmp",
  "strcpy",
  "strncpy",
  "strcat",
  "strchr",
  "strrchr",
  "strstr",
  "sprintf",
  "snprintf",
  "vsnprintf",
  "printf",
  "fprintf",
  // I/O
  "fwrite",
  "fread",
  "fopen",
  "fclose",
  "fflush",
  "fseek",
  "ftell",
  "rewind",
  "fgets",
  "fputs",
  "fgetc",
  "fputc",
  // C++ I/O
  "std::basic_ostream",
  "std::basic_istream",
  "std::basic_ofstream",
  "std::basic_ifstream",
  "std::ios_base",
  // C++ containers
  "std::vector",
  "std::map",
  "std::set",
  "std::unordered_map",
  "std::unordered_set",
  "std::basic_string",
  "std::shared_ptr",
  "std::unique_ptr",
  "std::allocator",
  // C++ algorithm / utility
  "std::sort",
  "std::find",
  "std::copy",
  "std::fill",
  "std::transform",
  "std::reverse",
  "std::min",
  "std::max",
  "std::pair",
  "std::make_pair",
  // libstdc++ internals
  "__gnu_cxx::__normal_iterator",
  "__cxxabiv1::__class_type_info",
  "__cxxabiv1::__si_class_type_info",
  "_ZSt", // mangled std:: prefix
  "_ZNSt", // mangled std:: nested
  // compiler helpers
  "__builtin_",
  "__cxa_",
  "_Unwind_",
  // libgcc
  "__muldi3",
  "__divdi3",
  "__moddi3",
  "__udivdi3",
  "__umoddi3",
  "__floatunsidf",
  "__fixdfsi",
  "__extendsfdf2",
  "__truncdfsf2",
  "__addsf3",
  "__subsf3",
  "__mulsf3",
  "__divsf3",
  "__gesf2",
  "__gtsf2",
  "__lesf2",
  "__ltsf2",
  "__eqsf2",
  "__nesf2",
  "__adddf3",
  "__subdf3",
  "__muldf3",
  "__divdf3",
  "__gedf2",
  "__gtdf2",
  "__ledf2",
  "__ltdf2",
  "__eqdf2",
  "__nedf2",
];

/** Mesa/Gallium/LLVM internal functions (hardware-independent rendering internals). */
const MESA_PREFIXES = [
  "lp_",
  "llvmpipe_",
  "draw_",
  "gallivm_",
  "tgsi_",
  "util_",
  "u_",
  "pb_",
  "pipe_",
  "_mesa_",
  "_glapi_",
  "st_",
  "nv50_",
  "r600_",
  "radeon_",
  "i915_",
  "i965_",
  "brw_",
  "iris_",
  "zink_",
  "llvm::",
  "LLVM",
  "LLVMTargetMachine",
  "LLVMCreate",
  "LLVMDispose",
  "LLVMAdd",
  "LLVMRemove",
  "LLVMBuild",
  "_ZNK4llvm",
];

/**
 * D3 rendering pipeline subsystem prefixes.
 * Each entry has a short name, regex pattern, display color, and full description.
 */
export interface D3Module {
  prefix: string;
  name: string;
  color: string;
  description: string;
}

export const D3_MODULES: D3Module[] = [
  { prefix: "g3_", name: "3D Pipeline", color: "#58a6ff", description: "g3_DrawPoly, g3_RotatePoint, g3_ProjectPoint, …" },
  { prefix: "rend_", name: "Render State", color: "#3fb950", description: "rend_SetAlphaType, rend_SetLighting, rend_DrawPoly…" },
  { prefix: "gpu_", name: "GPU Backend", color: "#d29922", description: "gpu_BindTexture, gpu_RenderPolygon, …" },
  { prefix: "mesa_", name: "Mesa Backend", color: "#bc8cff", description: "mesa_SetLogicalSize, mesa_rend_Init, …" },
  { prefix: "gl", name: "OpenGL API", color: "#f0883e", description: "glEnable, glDisable, glVertex3f, glFlush, …" },
  { prefix: "opengl_", name: "OpenGL Cache", color: "#ff7b72", description: "opengl_SetDefaults, opengl_ResetCache, …" },
  { prefix: "grtext_", name: "Text / HUD", color: "#79c0ff", description: "grtext_Flush, grtext_Printf, font rendering" },
  { prefix: "ddgr_", name: "2D Graphics", color: "#56d364", description: "ddgr_color, 2D drawing primitives" },
  { prefix: "bm_", name: "Bitmap", color: "#f85149", description: "bm_AllocLoadFileBitmap, texture management" },
  { prefix: "cf_", name: "File I/O", color: "#8b949e", description: "cf_ReadInt, cf_WriteByte, HOG file access" },
  { prefix: "osiris_", name: "Scripting", color: "#db6d28", description: "Osiris scripting system bindings" },
  { prefix: "mng_", name: "Table / Manage", color: "#3d888f", description: "mng_InitTableFiles, page management" },
];

/** Build a combined regex that matches any of the given prefixes. */
function prefixRe(prefixes: string[]): RegExp {
  const escaped = prefixes.map((p) =>
    p.replace(/[.*+?^${}()|[\]\\]/g, "\\$&")
  );
  return new RegExp(`^(${escaped.join("|")})`);
}

const RE_STDLIB = prefixRe(STDLIB_PREFIXES);
const RE_MESA = prefixRe(MESA_PREFIXES);
const RE_D3_MODULES = new RegExp(
  `^(${D3_MODULES.map((m) => m.prefix.replace(/[.*+?^${}()|[\]\\]/g, "\\$&")).join("|")})`
);

/** Classify a function name into a D3 module or null. */
export function classifyFn(fn: string): D3Module | null {
  for (const mod of D3_MODULES) {
    if (fn.startsWith(mod.prefix)) return mod;
  }
  return null;
}

/** True if the function is a C++ standard library / runtime helper. */
export function isStdlib(fn: string): boolean {
  return RE_STDLIB.test(fn);
}

/** True if the function is a Mesa/internal GPU driver helper. */
export function isMesaInternal(fn: string): boolean {
  return RE_MESA.test(fn);
}

// ── Filter options ────────────────────────────────────────────────────────

export interface FilterOptions {
  /** Hide C++ standard library / runtime functions. */
  hideStdlib: boolean;
  /** Hide Mesa/Gallium/LLVM internal GPU driver functions. */
  hideMesaInternal: boolean;
  /** Entry point function name (root of the call chain). */
  rootFn: string;
  /** Terminal function name (leaf of the call chain). */
  leafFn: string;
  /** Maximum call depth from root (0 = no limit). */
  maxDepth: number;
  /** Minimum cost fraction to keep a node (0.0–1.0). 0.001 = 0.1%. */
  minCostPct: number;
  /** Only show these D3 modules; empty = show all. */
  enabledModules: Set<string>;
  /** Collapse D3 subsystem groups into single cluster nodes. */
  groupByModule: boolean;
}

const DEFAULT_FILTER: FilterOptions = {
  hideStdlib: true,
  hideMesaInternal: true,
  rootFn: "main",
  leafFn: "glFlush",
  maxDepth: 10,
  minCostPct: 0.001,
  enabledModules: new Set(),
  groupByModule: true,
};

// ── Filter result ─────────────────────────────────────────────────────────

export interface FilterReport {
  totalNodes: number;
  totalEdges: number;
  removedStdlib: number;
  removedMesa: number;
  removedCost: number;
  removedDepth: number;
  keptNodes: number;
  moduleCounts: Record<string, number>;
  groups: Record<string, string[]>; // module name → list of function keys
}

// ── Graph helpers ─────────────────────────────────────────────────────────

function buildAdj(edges: GraphEdge[]): Map<string, string[]> {
  const adj = new Map<string, string[]>();
  for (const e of edges) {
    if (!adj.has(e.from)) adj.set(e.from, []);
    adj.get(e.from)!.push(e.to);
  }
  return adj;
}

function buildReverseAdj(edges: GraphEdge[]): Map<string, string[]> {
  const rev = new Map<string, string[]>();
  for (const e of edges) {
    if (!rev.has(e.to)) rev.set(e.to, []);
    rev.get(e.to)!.push(e.from);
  }
  return rev;
}

function bfsReachable(
  start: string,
  adj: Map<string, string[]>,
  maxDepth: number
): Map<string, number> {
  const visited = new Map<string, number>();
  const queue: string[] = [start];
  visited.set(start, 0);
  while (queue.length > 0) {
    const u = queue.shift()!;
    const d = visited.get(u)!;
    if (maxDepth > 0 && d >= maxDepth) continue;
    for (const v of adj.get(u) ?? []) {
      if (!visited.has(v)) {
        visited.set(v, d + 1);
        queue.push(v);
      }
    }
  }
  return visited;
}

// ── Main filter function ──────────────────────────────────────────────────

export function filterGraph(
  graph: CallGraph,
  options?: Partial<FilterOptions>
): { graph: CallGraph; report: FilterReport } {
  const opts: FilterOptions = { ...DEFAULT_FILTER, ...options };
  const { root, nodes, edges } = graph;

  const report: FilterReport = {
    totalNodes: Object.keys(nodes).length,
    totalEdges: edges.length,
    removedStdlib: 0,
    removedMesa: 0,
    removedCost: 0,
    removedDepth: 0,
    keptNodes: 0,
    moduleCounts: {},
    groups: {},
  };

  // Step 1: Classify all nodes
  const nodeClass = new Map<string, "stdlib" | "mesa" | "d3" | "other">();
  for (const key of Object.keys(nodes)) {
    if (opts.hideStdlib && isStdlib(key)) {
      nodeClass.set(key, "stdlib");
    } else if (opts.hideMesaInternal && isMesaInternal(key)) {
      nodeClass.set(key, "mesa");
    } else if (classifyFn(key)) {
      nodeClass.set(key, "d3");
    } else {
      nodeClass.set(key, "other");
    }
  }

  // Step 2: Name-based filtering (stdlib + mesa)
  const keepIds = new Set<string>();
  for (const key of Object.keys(nodes)) {
    const cls = nodeClass.get(key);
    if (cls === "stdlib") {
      report.removedStdlib++;
    } else if (cls === "mesa") {
      report.removedMesa++;
    } else {
      keepIds.add(key);
    }
  }

  // Step 3: Bidirectional reachability (root ∩ leaf)
  const fwd = bfsReachable(opts.rootFn, buildAdj(edges), opts.maxDepth);
  const rev = bfsReachable(opts.leafFn, buildReverseAdj(edges), 0);
  const reachableIds = new Set<string>();
  for (const id of keepIds) {
    if (fwd.has(id) && rev.has(id)) {
      reachableIds.add(id);
    }
  }

  // Step 4: Depth filtering
  const depthFilteredIds = new Set<string>();
  for (const id of reachableIds) {
    const d = fwd.get(id) ?? 0;
    if (opts.maxDepth > 0 && d > opts.maxDepth) {
      report.removedDepth++;
      continue;
    }
    depthFilteredIds.add(id);
  }

  // Step 5: Cost threshold
  const totalCost =
    Object.values(nodes).reduce((s, n) => s + n.total_cost, 0) || 1;
  const costThreshold = totalCost * opts.minCostPct;
  const costFilteredIds = new Set<string>();
  for (const id of depthFilteredIds) {
    const nd = nodes[id];
    if (nd.total_cost < costThreshold) {
      report.removedCost++;
      continue;
    }
    costFilteredIds.add(id);
  }

  // Step 6: Build filtered graph
  const finalIds = costFilteredIds;
  const keepEdgeIndices = new Set<number>();
  const filteredEdges: GraphEdge[] = [];
  for (let i = 0; i < edges.length; i++) {
    const e = edges[i];
    if (finalIds.has(e.from) && finalIds.has(e.to)) {
      keepEdgeIndices.add(i);
      filteredEdges.push(e);
    }
  }

  const filteredNodes: Record<string, GraphNode> = {};
  for (const id of finalIds) {
    filteredNodes[id] = nodes[id];
  }

  // Need to add root and leaf even if they were filtered
  for (const required of [opts.rootFn, opts.leafFn]) {
    if (nodes[required] && !filteredNodes[required]) {
      filteredNodes[required] = nodes[required];
    }
  }

  // Step 7: Module grouping (optional)
  if (opts.groupByModule) {
    return groupByModule(filteredNodes, filteredEdges, opts, report);
  }

  report.keptNodes = Object.keys(filteredNodes).length;

  // Count modules
  for (const id of Object.keys(filteredNodes)) {
    const mod = classifyFn(id);
    const modName = mod ? mod.name : "Other";
    report.moduleCounts[modName] = (report.moduleCounts[modName] || 0) + 1;
  }

  return {
    graph: { root: opts.rootFn, nodes: filteredNodes, edges: filteredEdges },
    report,
  };
}

// ── Module grouping ───────────────────────────────────────────────────────

function groupByModule(
  nodes: Record<string, GraphNode>,
  edges: GraphEdge[],
  opts: FilterOptions,
  report: FilterReport
): { graph: CallGraph; report: FilterReport } {
  // Assign each node to a module group
  const nodeToGroup = new Map<string, string>();
  const groupMembers = new Map<string, string[]>();

  for (const id of Object.keys(nodes)) {
    const mod = classifyFn(id);
    const groupName = mod ? mod.name : "Other";
    nodeToGroup.set(id, groupName);
    if (!groupMembers.has(groupName)) groupMembers.set(groupName, []);
    groupMembers.get(groupName)!.push(id);
    report.moduleCounts[groupName] = (report.moduleCounts[groupName] || 0) + 1;
  }

  // Create synthetic group nodes
  const groupedNodes: Record<string, GraphNode> = {};
  const moduleNameToGroupId = new Map<string, string>();

  for (const [groupName, members] of groupMembers) {
    const groupId = `[${groupName}]`;
    moduleNameToGroupId.set(groupName, groupId);

    // Aggregate costs from members
    let totalCost = 0;
    let selfCost = 0;
    let callsIn = 0;
    for (const id of members) {
      totalCost += nodes[id].total_cost;
      selfCost += nodes[id].self_cost;
      callsIn += nodes[id].calls_in;
    }

    groupedNodes[groupId] = {
      fn: groupId,
      short_fn: groupName,
      file: "",
      self_cost: selfCost,
      total_cost: totalCost,
      calls_in: callsIn,
      endpoint: members.length === 1 ? nodes[members[0]].endpoint : false,
    };

    report.groups[groupName] = members;
  }

  // Create edges between groups (collapse intra-group edges)
  const groupEdgeMap = new Map<string, number>(); // "from|to" → call count
  for (const e of edges) {
    const fromGroup = nodeToGroup.get(e.from) ?? "Other";
    const toGroup = nodeToGroup.get(e.to) ?? "Other";
    if (fromGroup === toGroup) continue; // skip intra-group edges
    const fromId = moduleNameToGroupId.get(fromGroup)!;
    const toId = moduleNameToGroupId.get(toGroup)!;
    const key = `${fromId}|${toId}`;
    groupEdgeMap.set(key, (groupEdgeMap.get(key) || 0) + e.calls);
  }

  const groupedEdges: GraphEdge[] = [];
  const groupIdList = Array.from(moduleNameToGroupId.values());

  for (const [key, calls] of groupEdgeMap) {
    const [from, to] = key.split("|");
    groupedEdges.push({
      from,
      to,
      calls,
      cost: calls, // use call count as rough cost when grouped
    });
  }

  // Also keep individual nodes that are in enabled modules
  if (opts.enabledModules.size > 0) {
    // If specific modules are requested, keep only those groups
    const keepGroups = new Set(opts.enabledModules);
    const filteredGrouped: Record<string, GraphNode> = {};
    for (const [id, nd] of Object.entries(groupedNodes)) {
      const groupName = id.slice(1, -1); // strip [brackets]
      if (keepGroups.has(groupName) || keepGroups.size === 0) {
        filteredGrouped[id] = nd;
      }
    }
    report.keptNodes = Object.keys(filteredGrouped).length;
    return {
      graph: {
        root: `[${classifyFn(opts.rootFn)?.name ?? "Other"}]`,
        nodes: filteredGrouped,
        edges: groupedEdges.filter(
          (e) => filteredGrouped[e.from] && filteredGrouped[e.to]
        ),
      },
      report,
    };
  }

  report.keptNodes = groupIdList.length;

  return {
    graph: {
      root: `[${classifyFn(opts.rootFn)?.name ?? "Other"}]`,
      nodes: groupedNodes,
      edges: groupedEdges,
    },
    report,
  };
}

// ── Utility: get module color ─────────────────────────────────────────────

export function getModuleColor(fn: string): string {
  const mod = classifyFn(fn);
  return mod?.color ?? "#8b949e";
}
