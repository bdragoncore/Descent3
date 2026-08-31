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
  baselineDir?: string;
  report: string;
  verbose: boolean;
  rebuild: boolean;
  noBuild: boolean;
  tracingFlag: boolean | null;
  updateBaseline: boolean;
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
  { name: "d3_render_tests_basic", description: "Basic render tests" },
  { name: "d3_render_tests_egl", description: "EGL context tests" },
  { name: "d3_render_tests_gl", description: "GL tests" },
  { name: "d3_render_tests_text", description: "Text/HUD render tests" },
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
    baselineDir: undefined,
    report: "render_report.html",
    verbose: false,
    rebuild: false,
    noBuild: false,
    tracingFlag: null,
    updateBaseline: false,
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
      case "--baseline-dir":
        args.baselineDir = next();
        break;
      case "--report":
        args.report = next() ?? args.report;
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
      case "--update-baseline":
        args.updateBaseline = true;
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
  } = {}
): Promise<CommandResult> {
  const { cwd, env, verbose } = options;

  if (verbose) {
    console.log(dim(`   $ ${cmd} ${args.join(" ")}`));
    if (cwd) console.log(dim(`   cwd=${cwd}`));
  }

  return await new Promise<CommandResult>((resolve, reject) => {
    const child = spawn(cmd, args, {
      cwd,
      env,
      stdio: verbose ? "inherit" : "pipe",
    });

    let stdout = "";
    let stderr = "";

    if (!verbose && child.stdout) {
      child.stdout.on("data", (d) => {
        stdout += d.toString();
      });
    }
    if (!verbose && child.stderr) {
      child.stderr.on("data", (d) => {
        stderr += d.toString();
      });
    }

    child.on("error", (err) => {
      reject(err);
    });

    child.on("close", (code) => {
      resolve({ code, stdout, stderr });
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

async function ensureCmakeConfigured(
  buildDir: string,
  projectRoot: string,
  verbose: boolean
): Promise<boolean> {
  const cachePath = path.join(buildDir, "CMakeCache.txt");
  if (existsSync(cachePath)) return true;

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
  onFinished?: (executable: string, durationMs: number) => void
): Promise<{
  tests: ParsedTestResult[];
  pngs: string[];
  md5s: string[];
  traces: string[];
  passed: boolean;
}> {
  const binaryPath = path.join(buildDir, "tests", executable);
  if (!existsSync(binaryPath)) {
    return { tests: [], pngs: [], md5s: [], traces: [], passed: false };
  }

  const fullEnv = {
    ...process.env,
    ...env,
    ...(tracing
      ? {
          TRACE_OUTPUT_DIR: path.resolve(outputDir),
          LD_BIND_NOW: "1",
        }
      : {}),
  };

  const start = Date.now();
  const res = await runCommand(binaryPath, [], {
    cwd: outputDir,
    env: fullEnv,
    verbose,
  });
  const durationMs = Date.now() - start;
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

async function main() {
  const args = parseArgs(process.argv.slice(2));

  const scriptDir = path.dirname(new URL(import.meta.url).pathname);
  const testsDir = path.resolve(scriptDir, "..");
  const projectRoot = path.resolve(testsDir, "..");

  const buildDir = path.resolve(projectRoot, args.buildDir);
  const outputDir =
    args.outputDir ??
    path.join(buildDir, "tests", "render_output");
  const baselineDir =
    args.baselineDir ?? path.join(testsDir, "render_output_baseline");

  const tracingAvailable = await detectTracingAvailable(buildDir);
  const tracingEnabled =
    args.tracingFlag !== null ? args.tracingFlag : tracingAvailable;
  const callgrindAvailable = await checkValgrindAvailable();

  console.log("\n" + purple.bold("Render Test Runner (TypeScript)\n"));
  const kv = (k: string, v: string) =>
    console.log(dim(k.padEnd(20) + v));
  kv("Build directory:", buildDir);
  kv("Output directory:", outputDir);
  kv("Baseline directory:", baselineDir);
  kv(
    "Tracing:",
    `${tracingEnabled ? "enabled" : "disabled"} (${tracingAvailable ? "available" : "not available"})`
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
    for (const bin of RENDER_BINARIES) {
      const binaryPath = path.join(buildDir, "tests", bin.name);
      status.update(`Running ${bin.name} (${bin.description})...`);
      status.update(`$ ${binaryPath}`);
      const env = { SDL_VIDEODRIVER: "offscreen" };
      const exec = await runTestsExecutable(
        bin.name,
        buildDir,
        env,
        outputDir,
        args.verbose,
        tracingEnabled,
        (exe, durationMs) => status.update(`${exe} finished in ${durationMs}ms`)
      );

      if (!exec.tests.length) {
        status.log(red("   No tests found or execution failed"));
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
        if (t.status === "PASSED") {
          totalPassed += 1;
          status.log(`   ${green("✓")} ${line}`);
        } else {
          totalFailed += 1;
          status.log(`   ${red("✗")} ${line}`);
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
      const cliPath = path.join(
        testsDir,
        "report_tools",
        "callgraph_cli.py"
      );
      const argsPy = [cliPath, callgraphPath!, ...allTracePaths];
      const res = await runCommand("python3", argsPy, {
        cwd: projectRoot,
        verbose: args.verbose,
      });
      if (res.code !== 0) {
        throw new Error(
          `callgraph_cli.py failed with code ${res.code}${
            res.stderr ? `: ${res.stderr}` : ""
          }`
        );
      }
      console.log(dim(`   Call graph: ${callgraphPath}`));
    });
  }

  if (tracingEnabled && allTracePaths.length) {
    await runTask("Generate per-trace HTML", async () => {
      const genPath = path.join(
        testsDir,
        "report_tools",
        "generate_html.py"
      );
      for (const trace of allTracePaths) {
        const res = await runCommand("python3", [genPath, trace], {
          cwd: projectRoot,
          verbose: args.verbose,
        });
        if (res.code !== 0) {
          console.log(
            red(`  Error generating HTML for ${path.basename(trace)}`)
          );
          if (!args.verbose && res.stderr) console.log(dim(res.stderr));
        } else if (args.verbose) {
          const htmlName = path.basename(trace).replace(".json", ".html");
          console.log(dim(`   Generated: ${htmlName}`));
        }
      }
      if (allTracePaths.length) {
        console.log(dim(`   Generated ${allTracePaths.length} trace HTML files`));
      }
    });
  }

  await runTask("Generate HTML report", async () => {
    const resultsJson = path.join(outputDir, "render_results.json");
    await fs.writeFile(resultsJson, JSON.stringify(allResults, null, 2), "utf8");

    const cliPath = path.join(
      testsDir,
      "report_tools",
      "html_report_cli.py"
    );

    const baselineArg = existsSync(baselineDir) ? baselineDir : "NONE";
    const callgraphArg = callgraphPath && existsSync(callgraphPath)
      ? callgraphPath
      : "NONE";
    const viewerBaseUrl = args.serve
      ? `http://localhost:${args.port}`
      : "NONE";

    const res = await runCommand(
      "python3",
      [
        cliPath,
        resultsJson,
        outputDir,
        baselineArg,
        args.report,
        args.updateBaseline ? "1" : "0",
        tracingEnabled ? "1" : "0",
        callgraphArg,
        viewerBaseUrl,
      ],
      { cwd: projectRoot, verbose: args.verbose }
    );
    if (res.code !== 0) {
      throw new Error(
        `html_report_cli.py failed with code ${res.code}${
          res.stderr ? `: ${res.stderr}` : ""
        }`
      );
    }
    console.log(dim(`   Report: ${path.join(outputDir, args.report)}`));
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
  const reportPath = path.join(outputDir, args.report);
  const reportStr =
    reportPath.length > summaryWidth - 8
      ? "…" + reportPath.slice(-(summaryWidth - 9))
      : reportPath;
  console.log(
    green.bold("║") + dim(pad("Report: " + reportStr)) + green.bold("║")
  );
  console.log(green.bold("╚" + border + "╝"));
  console.log();

  process.exitCode = totalFailed === 0 ? 0 : 1;
}

main().catch((err) => {
  console.error(red("Unexpected crash:"), err);
  process.exit(1);
});

