import path from "node:path";
import fs from "node:fs/promises";
import { existsSync } from "node:fs";
import { spawn } from "node:child_process";
import { setTimeout as delay } from "node:timers/promises";
import ora from "ora";
import chalk from "chalk";

const purple = chalk.hex("#bb00ff");
const green = chalk.green.bold;
const red = chalk.red.bold;
const dim = chalk.dim;

type RenderArgs = {
  buildDir: string;
  outputDir?: string;
  verbose: boolean;
  rebuild: boolean;
  noBuild: boolean;
  tracingFlag: boolean | null;
  skipTrace: boolean;
  serve: boolean;
  port: number;
};

type ParsedTestResult = {
  name: string;
  suite: string;
  status: "PASSED" | "FAILED";
  duration_ms: number;
};

type AllResult = {
  exe: string;
  test_name: string;
  test_suite: string;
  passed: boolean;
  duration_ms: number;
  pngs: string[];
  md5s: string[];
  traces: string[];
  environment: Record<string, string>;
  callgraphs: string[];
};

type CommandResult = {
  code: number | null;
  stdout: string;
  stderr: string;
};

const RENDER_BINARIES = [
  { name: "d3_render_tests_game", description: "D3 game rendering (polymodel, etc.)" },
] as const;

const SPINNER_FRAMES = ["⠋", "⠙", "⠹", "⠸", "⠼", "⠴", "⠦", "⠧", "⠇", "⠏"];
const SPINNER_INTERVAL_MS = 80;

type BottomStatusLine = {
  log: (line: string) => void;
  update: (text: string) => void;
  stop: () => void;
};

function createBottomStatusLine(
  stepLabel: string,
  out: NodeJS.WriteStream = process.stdout
): BottomStatusLine {
  let statusText = stepLabel;
  let frameIndex = 0;
  let intervalId: ReturnType<typeof setInterval> | null = null;
  const stepStyled = purple(stepLabel);

  const draw = () => {
    const frame = SPINNER_FRAMES[frameIndex % SPINNER_FRAMES.length];
    const line = `${frame} ${stepStyled}${statusText !== stepLabel ? dim("  " + statusText) : ""}`;
    out.write("\r\x1b[2K" + line);
  };

  const startInterval = () => {
    if (intervalId) return;
    intervalId = setInterval(() => {
      frameIndex += 1;
      draw();
    }, SPINNER_INTERVAL_MS);
  };

  const stopInterval = () => {
    if (intervalId) {
      clearInterval(intervalId);
      intervalId = null;
    }
  };

  draw();
  startInterval();

  return {
    log(line: string) {
      stopInterval();
      out.write("\r\x1b[2K" + line + "\n");
      statusText = stepLabel; // reset so draw doesn't show stale "Running X..."
      draw();
      startInterval();
    },
    update(text: string) {
      statusText = text;
    },
    stop() {
      stopInterval();
    },
  };
}

async function runTask(
  name: string,
  optsOrTask: { statusLine: true } | (() => Promise<void> | void),
  taskFn?: (status: BottomStatusLine) => Promise<void> | void
): Promise<void> {
  const useStatusLine = typeof optsOrTask === "object" && optsOrTask.statusLine;
  const task = typeof optsOrTask === "function" ? optsOrTask : taskFn!;
  const label = chalk.bold.white(name);

  if (useStatusLine) {
    const statusLine = createBottomStatusLine(name);
    const start = Date.now();
    try {
      await (task as (s: BottomStatusLine) => Promise<void> | void)(statusLine);
      const ms = Date.now() - start;
      statusLine.stop();
      process.stdout.write("\r\x1b[2K" + green("✔") + " " + green(label) + dim(` (${ms}ms)\n`));
    } catch (err: any) {
      statusLine.stop();
      process.stdout.write("\r\x1b[2K" + red("✗") + " " + red(label) + "\n");
      console.error(dim(`   → ${err?.message ?? String(err)}`));
      process.exitCode = 1;
      throw err;
    }
    return;
  }

  const spinner = ora({
    text: purple(`▶  ${label}`),
    color: "magenta",
    spinner: "dots",
    indent: 0,
  }).start();

  const start = Date.now();

  try {
    await (task as () => Promise<void> | void)();
    const ms = Date.now() - start;
    spinner.succeed(green(label) + dim(` (${ms}ms)`));
  } catch (err: any) {
    spinner.fail(red(label));
    console.error(dim(`   → ${err?.message ?? String(err)}`));
    process.exitCode = 1;
    throw err;
  }
}

function parseArgs(argv: string[]): RenderArgs {
  const args: RenderArgs = {
    buildDir: "build",
    outputDir: undefined,
    verbose: false,
    rebuild: false,
    noBuild: false,
    tracingFlag: null,
    skipTrace: false,
    serve: false,
    port: 3000,
  };

  for (let i = 0; i < argv.length; i++) {
    const arg = argv[i];
    const next = () => argv[++i];
    switch (arg) {
      case "--build-dir":
        args.buildDir = next() ?? args.buildDir;
        break;
      case "--output-dir":
        args.outputDir = next();
        break;
      case "--verbose":
      case "-v":
        args.verbose = true;
        break;
      case "--rebuild":
        args.rebuild = true;
        break;
      case "--no-build":
        args.noBuild = true;
        break;
      case "--tracing":
        args.tracingFlag = true;
        break;
      case "--skip-trace":
        args.skipTrace = true;
        args.tracingFlag = false;
        break;
      case "--serve":
        args.serve = true;
        break;
      case "--port": {
        const v = next();
        if (v) args.port = parseInt(v, 10) || args.port;
        break;
      }
      default:
        break;
    }
  }

  return args;
}

async function runCommand(
  cmd: string,
  args: string[],
  options: {
    cwd?: string;
    env?: NodeJS.ProcessEnv;
    verbose?: boolean;
    timeoutMs?: number;
  } = {}
): Promise<CommandResult> {
  const { cwd, env, verbose, timeoutMs } = options;

  if (verbose) {
    console.log(dim(`   $ ${cmd} ${args.join(" ")}`));
    if (cwd) console.log(dim(`   cwd=${cwd}`));
  }

  return await new Promise<CommandResult>((resolve, reject) => {
    const child = spawn(cmd, args, {
      cwd,
      env,
      stdio: "pipe", // always pipe so we can parse results (even when verbose)
    });

    let stdout = "";
    let stderr = "";

    if (child.stdout) {
      child.stdout.on("data", (d) => {
        const s = d.toString();
        stdout += s;
        if (verbose) process.stdout.write(s);
      });
    }
    if (child.stderr) {
      child.stderr.on("data", (d) => {
        const s = d.toString();
        stderr += s;
        if (verbose) process.stderr.write(s);
      });
    }

    let settled = false;
    const settle = (code: number | null, out: string, err: string) => {
      if (settled) return;
      settled = true;
      if (timeoutId !== undefined) clearTimeout(timeoutId);
      resolve({ code, stdout: out, stderr: err });
    };

    let timeoutId: ReturnType<typeof setTimeout> | undefined;
    if (timeoutMs != null && timeoutMs > 0) {
      timeoutId = setTimeout(() => {
        if (settled) return;
        settled = true;
        child.kill("SIGTERM");
        const msg = `[runner] timeout after ${timeoutMs}ms, killed ${cmd}`;
        resolve({
          code: null,
          stdout,
          stderr: stderr + "\n" + msg,
        });
      }, timeoutMs);
    }

    child.on("error", (err) => {
      if (settled) return;
      settled = true;
      if (timeoutId !== undefined) clearTimeout(timeoutId);
      reject(err);
    });

    child.on("close", (code) => {
      settle(code ?? null, stdout, stderr);
    });
  });
}

async function detectTracingAvailable(buildDir: string): Promise<boolean> {
  const cachePath = path.join(buildDir, "CMakeCache.txt");
  if (!existsSync(cachePath)) return false;
  const content = await fs.readFile(cachePath, "utf8");
  if (!content.toLowerCase().includes("clang")) return false;
  const tracerLib = path.join(buildDir, "tests", "libtracer.a");
  return existsSync(tracerLib);
}

async function clearPreviousArtifacts(outputDir: string): Promise<void> {
  if (!existsSync(outputDir)) {
    return;
  }
  const entries = await fs.readdir(outputDir);
  const toDelete = entries.filter((name) => {
    if (name.startsWith("Test") && (name.endsWith(".png") || name.endsWith(".md5"))) {
      return true;
    }
    if (name.startsWith("trace__") && name.endsWith(".json")) {
      return true;
    }
    if (name === "render_results.json" || name === "render_report.html" || name === "callgraph.json") {
      return true;
    }
    return false;
  });
  await Promise.all(
    toDelete.map(async (name) => {
      const full = path.join(outputDir, name);
      try {
        await fs.unlink(full);
      } catch {
        // Best-effort cleanup; ignore failures so tests still run.
      }
    })
  );
}

async function ensureCmakeConfigured(
  buildDir: string,
  projectRoot: string,
  verbose: boolean
): Promise<boolean> {
  const cachePath = path.join(buildDir, "CMakeCache.txt");
  
  // Check if CMake is configured AND build files exist
  if (existsSync(cachePath)) {
    // Verify build files exist (Makefile for Unix Makefiles generator)
    const makePath = path.join(buildDir, "Makefile");
    const ninjaPath = path.join(buildDir, "build.ninja");
    if (existsSync(makePath) || existsSync(ninjaPath)) {
      return true;
    }
    // CMakeCache exists but build files are missing - need reconfigure
  }

  const res = await runCommand(
    "cmake",
    ["-B", buildDir, "-DBUILD_TESTING=ON"],
    { cwd: projectRoot, verbose }
  );
  if (res.code !== 0) {
    console.error(red("CMake configuration failed"));
    if (!verbose && res.stderr) {
      console.error(dim(res.stderr));
    }
    return false;
  }
  return true;
}

async function ensureBuilt(
  buildDir: string,
  target: string,
  force: boolean,
  verbose: boolean
): Promise<boolean> {
  const binaryPath = path.join(buildDir, "tests", target);
  if (!force && existsSync(binaryPath)) return true;

  const res = await runCommand(
    "cmake",
    ["--build", buildDir, "--target", target],
    { verbose }
  );
  if (res.code !== 0) {
    console.error(red(`Build failed for ${target}`));
    if (!verbose && res.stderr) {
      console.error(dim(res.stderr));
    }
    return false;
  }
  return true;
}

function parseGTestOutput(output: string): ParsedTestResult[] {
  const results: ParsedTestResult[] = [];
  const okPattern = /\[\s+OK\s+]\s+(\w+)\.(\w+)\s+\((\d+)\s+ms\)/g;
  const failedPattern = /\[\s+FAILED\s+]\s+(\w+)\.(\w+)\s+\((\d+)\s+ms\)/g;

  let m: RegExpExecArray | null;
  while ((m = okPattern.exec(output))) {
    results.push({
      suite: m[1],
      name: m[2],
      duration_ms: parseInt(m[3], 10),
      status: "PASSED",
    });
  }
  while ((m = failedPattern.exec(output))) {
    results.push({
      suite: m[1],
      name: m[2],
      duration_ms: parseInt(m[3], 10),
      status: "FAILED",
    });
  }
  return results;
}

async function checkValgrindAvailable(): Promise<boolean> {
  try {
    const res = await runCommand("valgrind", ["--version"]);
    return res.code === 0;
  } catch {
    return false;
  }
}

async function runTestsExecutable(
  executable: string,
  buildDir: string,
  env: Record<string, string>,
  outputDir: string,
  verbose: boolean,
  tracing: boolean,
  binaryArgs: string[],
  gtestFilter?: string,
  onFinished?: (executable: string, durationMs: number) => void
): Promise<{
  tests: ParsedTestResult[];
  pngs: string[];
  md5s: string[];
  traces: string[];
  passed: boolean;
  timedOut: boolean;
}> {
  const binaryPath = path.join(buildDir, "tests", executable);
  if (!existsSync(binaryPath)) {
    return { tests: [], pngs: [], md5s: [], traces: [], passed: false, timedOut: false };
  }

  const fullEnv = {
    ...process.env,
    ...env,
    D3_SKIP_OVERLAY: "1",
    ...(tracing
      ? {
          TRACE_OUTPUT_DIR: path.resolve(outputDir),
          LD_BIND_NOW: "1",
        }
      : {}),
  };

  const start = Date.now();
  const effectiveArgs = [...binaryArgs];
  if (gtestFilter) {
    effectiveArgs.push(`--gtest_filter=${gtestFilter}`);
  }

  const res = await runCommand(binaryPath, effectiveArgs, {
    cwd: outputDir,
    env: fullEnv,
    verbose,
    timeoutMs: 2 * 60 * 1000,
  });
  const durationMs = Date.now() - start;
  const timedOut = res.code === null && res.stderr.includes("[runner] timeout");
  if (onFinished) {
    onFinished(executable, durationMs);
  } else if (!verbose) {
    console.log(dim(`   d3_render_tests finished in ${durationMs}ms`));
  }
  const tests = parseGTestOutput(res.stdout + res.stderr);

  const entries = await fs.readdir(outputDir);
  const pngs = entries
    .filter((f) => f.startsWith("Test") && f.endsWith(".png"))
    .map((f) => path.join(outputDir, f));
  const md5s = entries
    .filter((f) => f.startsWith("Test") && f.endsWith(".md5"))
    .map((f) => path.join(outputDir, f));
  const traces =
    tracing
      ? entries
          .filter((f) => f.startsWith("trace__") && f.endsWith(".json"))
          .map((f) => path.join(outputDir, f))
      : [];

  return {
    tests,
    pngs,
    md5s,
    traces,
    passed: res.code === 0,
    timedOut,
  };
}

async function startViewer(outputDir: string, port: number, verbose: boolean) {
  const viewerDir = path.resolve(
    path.dirname(new URL(import.meta.url).pathname),
    "..",
    "callgraph-viewer"
  );
  if (!existsSync(path.join(viewerDir, "package.json"))) {
    console.log(dim("[viewer] callgraph-viewer not found, skipping"));
    return;
  }

  const env = {
    ...process.env,
    OUTPUT_DIR: path.resolve(outputDir),
  };

  console.log(dim(`[viewer] starting Next.js viewer on http://localhost:${port}`));
  const child = spawn(
    "npm",
    ["run", "dev", "--", "-p", String(port)],
    {
      cwd: viewerDir,
      env,
      stdio: verbose ? "inherit" : "ignore",
      detached: true,
    }
  );
  child.unref();
}

function escapeHtml(s: string): string {
  return s
    .replace(/&/g, "&amp;")
    .replace(/</g, "&lt;")
    .replace(/>/g, "&gt;")
    .replace(/"/g, "&quot;");
}

function generateReportHtml(
  results: AllResult[],
  generatedAt: Date,
  htmxJs: string
): string {
  const passed = results.filter((r) => r.passed).length;
  const failed = results.length - passed;
  const total = results.length;
  const passPct = total > 0 ? Math.round((passed / total) * 100) : 0;

  const cards = results
    .map((r) => {
      const name = `${r.test_suite}.${r.test_name}`;
      const imgs = r.pngs
        .map(
          (p) =>
            `<img src="${escapeHtml(path.basename(p))}" loading="lazy" ` +
            `hx-on:click="this.classList.toggle('zoom')" alt="${escapeHtml(r.test_name)}">`
        )
        .join("\n");
      const traces = r.traces
        .map(
          (t) =>
            `<a class="trace" href="${escapeHtml(path.basename(t))}" target="_blank">trace</a>`
        )
        .join(" ");
      return (
        `<article class="card ${r.passed ? "pass" : "fail"}" data-name="${escapeHtml(name.toLowerCase())}">` +
        `<header><span class="tname">${escapeHtml(name)}</span>` +
        `<span class="badge ${r.passed ? "pass" : "fail"}">${r.passed ? "PASSED" : "FAILED"}</span></header>` +
        `<div class="meta">${escapeHtml(r.exe)} &middot; ${r.duration_ms}ms ${traces}</div>` +
        (imgs ? `<div class="imgs">\n${imgs}\n</div>` : "") +
        `</article>`
      );
    })
    .join("\n");

  return `<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>Render Test Report</title>
<style>
  :root { color-scheme: dark; }
  * { box-sizing: border-box; }
  body {
    font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif;
    background: #0f1117; color: #f1f5f9; margin: 0; padding: 24px; line-height: 1.5;
  }
  .container { max-width: 1400px; margin: 0 auto; }
  h1 { border-bottom: 2px solid #8b5cf6; padding-bottom: 12px; }
  .stats { display: grid; grid-template-columns: repeat(auto-fit, minmax(160px, 1fr)); gap: 16px; margin: 20px 0; }
  .stat { background: #161b22; border: 1px solid #2d3748; border-radius: 12px; padding: 16px; text-align: center; }
  .stat .n { font-size: 2rem; font-weight: 700; }
  .stat .l { font-size: 0.75rem; text-transform: uppercase; letter-spacing: 1px; color: #94a3b8; }
  .pass .n { color: #56d364; } .fail .n { color: #ff7b72; } .total .n { color: #f1f5f9; }
  .controls { display: flex; gap: 12px; margin: 20px 0; flex-wrap: wrap; }
  .controls input {
    flex: 1; min-width: 220px; background: #161b22; border: 1px solid #2d3748;
    color: #f1f5f9; border-radius: 8px; padding: 10px 14px; font-size: 0.95rem;
  }
  button {
    background: #21262d; color: #f1f5f9; border: 1px solid #2d3748; border-radius: 8px;
    padding: 10px 18px; font-size: 0.9rem; cursor: pointer;
  }
  button:hover { background: #2d333b; }
  button.active { background: #8b5cf6; border-color: #8b5cf6; color: #fff; }
  .grid { display: grid; grid-template-columns: repeat(auto-fill, minmax(420px, 1fr)); gap: 20px; }
  .card { background: #161b22; border: 1px solid #2d3748; border-radius: 14px; padding: 18px; }
  .card.pass { border-left: 4px solid #56d364; }
  .card.fail { border-left: 4px solid #ff7b72; }
  .card header { display: flex; justify-content: space-between; align-items: center; margin-bottom: 6px; }
  .card .tname { font-weight: 600; }
  .badge {
    padding: 4px 12px; border-radius: 999px; font-size: 0.75rem; font-weight: 700;
    text-transform: uppercase; letter-spacing: 1px;
  }
  .badge.pass { background: rgba(35, 134, 54, 0.25); color: #56d364; border: 1px solid #56d364; }
  .badge.fail { background: rgba(218, 54, 51, 0.25); color: #ff7b72; border: 1px solid #ff7b72; }
  .meta { color: #94a3b8; font-size: 0.85rem; margin-bottom: 12px; }
  .trace { color: #58a6ff; margin-left: 8px; }
  .imgs { display: grid; grid-template-columns: repeat(auto-fit, minmax(200px, 1fr)); gap: 10px; }
  .imgs img {
    width: 100%; border-radius: 8px; border: 1px solid #2d3748; cursor: zoom-in;
    image-rendering: pixelated; background: #000;
  }
  .imgs img.zoom {
    position: fixed; inset: 0; width: 100vw; height: 100vh; object-fit: contain;
    background: rgba(0, 0, 0, 0.95); z-index: 100; border: none; border-radius: 0; cursor: zoom-out;
  }
  .hidden { display: none; }
  body[data-filter="pass"] .card.fail { display: none; }
  body[data-filter="fail"] .card.pass { display: none; }
</style>
</head>
<body>
<div class="container">
  <h1>Render Test Report</h1>
  <p style="color:#94a3b8;font-size:0.85rem">Generated: ${escapeHtml(generatedAt.toLocaleString())} &middot; ${total} tests &middot; ${passPct}% passed</p>
  <div class="stats">
    <div class="stat total"><div class="n">${total}</div><div class="l">Total</div></div>
    <div class="stat pass"><div class="n">${passed}</div><div class="l">Passed</div></div>
    <div class="stat fail"><div class="n">${failed}</div><div class="l">Failed</div></div>
  </div>
  <div class="controls">
    <input id="search" type="search" placeholder="Filter tests..." autocomplete="off"
           hx-on:input="document.querySelectorAll('.card').forEach(c => c.classList.toggle('hidden', this.value && c.dataset.name.indexOf(this.value.toLowerCase()) < 0))">
    <button hx-on:click="document.body.dataset.filter='all'; document.querySelectorAll('.controls button').forEach(b => b.classList.remove('active')); this.classList.add('active')">All</button>
    <button hx-on:click="document.body.dataset.filter='pass'; document.querySelectorAll('.controls button').forEach(b => b.classList.remove('active')); this.classList.add('active')">Passed</button>
    <button hx-on:click="document.body.dataset.filter='fail'; document.querySelectorAll('.controls button').forEach(b => b.classList.remove('active')); this.classList.add('active')">Failed</button>
  </div>
  <div class="grid">
${cards}
  </div>
</div>
<script>${htmxJs}</script>
</body>
</html>
`;
}

async function main() {
  const args = parseArgs(process.argv.slice(2));

  const scriptDir = path.dirname(new URL(import.meta.url).pathname);
  const testsDir = path.resolve(scriptDir, "..");
  const projectRoot = path.resolve(testsDir, "..");

  const buildDir = path.resolve(projectRoot, args.buildDir);
  const outputDir = args.outputDir
    ? path.resolve(args.outputDir)
    : path.join(buildDir, "tests", "render_output");

  const tracingAvailable = await detectTracingAvailable(buildDir);
  // Tracing is opt-in: only enable when --tracing is passed (default: off for speed).
  const tracingEnabled = args.tracingFlag === true;
  const callgrindAvailable = await checkValgrindAvailable();

  console.log("\n" + purple.bold("Render Test Runner (TypeScript)\n"));
  const kv = (k: string, v: string) =>
    console.log(dim(k.padEnd(20) + v));
  kv("Build directory:", buildDir);
  kv("Output directory:", outputDir);
  kv(
    "Tracing:",
    tracingEnabled
      ? "enabled (--tracing)"
      : `disabled (default; use --tracing to enable, ${tracingAvailable ? "available" : "not available"})`
  );
  kv("Callgrind:", callgrindAvailable ? "enabled" : "disabled");
  console.log();

  if (args.tracingFlag && !tracingAvailable) {
    console.log(
      red(
        "Warning: --tracing specified but tracing not available (no clang or tracer library)"
      )
    );
  }

  await fs.mkdir(outputDir, { recursive: true });
  await clearPreviousArtifacts(outputDir);

  if (!args.noBuild) {
    await runTask("Configure CMake", async () => {
      const ok = await ensureCmakeConfigured(buildDir, projectRoot, args.verbose);
      if (!ok) throw new Error("CMake configuration failed");
    });

    await runTask("Build render test binaries", async () => {
      for (const bin of RENDER_BINARIES) {
        const ok = await ensureBuilt(
          buildDir,
          bin.name,
          args.rebuild,
          args.verbose
        );
        if (!ok) {
          throw new Error(`Build failed for ${bin.name}`);
        }
      }
    });
  }

  const allResults: AllResult[] = [];
  let totalPassed = 0;
  let totalFailed = 0;

  await runTask("Run render tests", { statusLine: true }, async (status) => {
    status.log(dim("   (each suite may take several minutes with software rendering; 2min timeout per suite)"));
    for (const bin of RENDER_BINARIES) {
      const binaryPath = path.join(buildDir, "tests", bin.name);
      status.update(`Running ${bin.name} (${bin.description})...`);
      status.update(`$ ${binaryPath}`);
      const env = { SDL_VIDEODRIVER: "offscreen" };
      // Pass --skip-trace to the binary whenever we're not collecting traces, so the
      // C++ doesn't run the trace listener (and stays fast).
      const binaryArgs = !tracingEnabled ? ["--skip-trace"] : [];
      const exec = await runTestsExecutable(
        bin.name,
        buildDir,
        env,
        outputDir,
        args.verbose,
        tracingEnabled,
        binaryArgs,
        undefined,
        (exe, durationMs) => status.update(`${exe} finished in ${durationMs}ms`)
      );

      if (!exec.tests.length) {
        if (exec.timedOut) {
          status.log(red("   Timed out after 2 minutes (render tests use software GL; try --verbose to see progress)"));
        } else {
          status.log(red("   No tests found or execution failed"));
        }
        continue;
      }

      status.log(dim(`   ${exec.tests.length} tests found`));
      for (const t of exec.tests) {
        // Filter PNGs for this specific test based on new naming pattern
        // PNG format: Test{Suite}_{Test}_{custom}.png
        const testPngPattern = `Test${t.suite}_${t.name}_`;
        const testPngs = exec.pngs.filter((png) =>
          path.basename(png).startsWith(testPngPattern)
        );

        allResults.push({
          exe: bin.name,
          test_name: t.name,
          test_suite: t.suite,
          passed: t.status === "PASSED",
          duration_ms: t.duration_ms,
          pngs: testPngs,
          md5s: exec.md5s,
          traces: exec.traces,
          environment: env,
          callgraphs: [],
        });
        const line = dim(`${t.suite}.${t.name} (${t.duration_ms}ms)`);
        const statusIcon = t.status === "PASSED" ? green("✓") : red("✗");

        if (t.status === "PASSED") {
          totalPassed += 1;
          status.log(`   ${statusIcon} ${line}`);
        } else {
          totalFailed += 1;
          status.log(`   ${statusIcon} ${line}`);
        }
      }
    }
  });

  let callgraphPath: string | null = null;
  const allTracePaths =
    tracingEnabled && existsSync(outputDir)
      ? (await fs.readdir(outputDir))
          .filter((f) => f.startsWith("trace__") && f.endsWith(".json"))
          .map((f) => path.join(outputDir, f))
      : [];

  if (tracingEnabled) {
    await runTask("Build aggregate call graph", async () => {
      callgraphPath = path.join(buildDir, "callgraph.json");
      const reportToolsDir = path.join(testsDir, "report-tools-ts");
      
      // Ensure dependencies are installed
      const nodeModulesPath = path.join(reportToolsDir, "node_modules");
      if (!existsSync(nodeModulesPath)) {
        console.log(dim("   Installing report tools dependencies..."));
        const installRes = await runCommand("npm", ["install"], {
          cwd: reportToolsDir,
          verbose: args.verbose,
        });
        if (installRes.code !== 0) {
          throw new Error(
            `Failed to install report tools dependencies: ${installRes.stderr || ""}`
          );
        }
      }
      
      // Generate callgraph using TypeScript (via tsx, no build needed)
      const traceArgs = allTracePaths.flatMap(t => ["--traces", t]);
      const res = await runCommand(
        "npm", 
        [
          "run", "generate", "--",
          "--mode", "trace",
          "--output-dir", path.dirname(callgraphPath!),
          ...traceArgs
        ], 
        {
          cwd: reportToolsDir,
          verbose: args.verbose,
        }
      );
      if (res.code !== 0) {
        throw new Error(
          `Call graph generation failed with code ${res.code}${
            res.stderr ? `: ${res.stderr}` : ""
          }`
        );
      }
      console.log(dim(`   Call graph: ${callgraphPath}`));
    });
  }

  await runTask("Write results JSON", async () => {
    const resultsJson = path.join(outputDir, "render_results.json");
    await fs.writeFile(resultsJson, JSON.stringify(allResults, null, 2), "utf8");
    console.log(dim(`   Results: ${resultsJson}`));
  });

  await runTask("Generate render report", async () => {
    const htmxPath = path.join(scriptDir, "htmx.min.js");
    let htmxJs: string;
    try {
      htmxJs = await fs.readFile(htmxPath, "utf8");
    } catch {
      throw new Error(`htmx.min.js not found at ${htmxPath} (run: curl -o tests/render/htmx.min.js https://unpkg.com/htmx.org@2.0.4/dist/htmx.min.js)`);
    }
    const reportPath = path.join(outputDir, "render_report.html");
    await fs.writeFile(reportPath, generateReportHtml(allResults, new Date(), htmxJs), "utf8");
    console.log(dim(`   Report: ${reportPath}`));
  });

  if (args.serve) {
    await runTask("Start call graph viewer", async () => {
      await startViewer(outputDir, args.port, args.verbose);
      await delay(500);
    });
  }

  const total = totalPassed + totalFailed;
  const summaryWidth = 50;
  const pad = (s: string) => s.padEnd(summaryWidth).slice(0, summaryWidth);
  const border = "═".repeat(summaryWidth);
  console.log();
  console.log(green.bold("╔" + border + "╗"));
  console.log(
    green.bold("║") + dim(pad(`Total: ${total}  Passed: ${totalPassed}  Failed: ${totalFailed}`)) + green.bold("║")
  );
  const resultsPath = path.join(outputDir, "render_results.json");
  const resultsStr =
    resultsPath.length > summaryWidth - 10
      ? "…" + resultsPath.slice(-(summaryWidth - 11))
      : resultsPath;
  console.log(
    green.bold("║") + dim(pad("Results: " + resultsStr)) + green.bold("║")
  );
  console.log(green.bold("╚" + border + "╝"));
  console.log();

  process.exitCode = totalFailed === 0 ? 0 : 1;
}

main().catch((err) => {
  console.error(red("Unexpected crash:"), err);
  process.exit(1);
});

