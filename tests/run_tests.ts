#!/usr/bin/env tsx
/**
 * Run all tests: unit tests first, then render tests.
 * Uses the same --build-dir (and optionally --no-build, --verbose) for both.
 *
 * Usage:
 *   npm run tests [-- --no-build] [-- --verbose] [-- --tracing]
 *   --tracing  enable render test tracing (default: off)
 *
 * Output:
 *   build-dir/tests/tests_report.html  — combined report with Unit/Render tabs
 *   build-dir/tests/render_output/     — PNGs, traces, etc.
 */

import path from "node:path";
import { stat } from "node:fs/promises";
import { spawn } from "node:child_process";

function parseArgs(argv: string[]): {
  buildDir: string;
  noBuild: boolean;
  verbose: boolean;
  tracing: boolean;
} {
  const args = {
    buildDir: "build",
    noBuild: false,
    verbose: false,
    tracing: false,
  };

  for (let i = 0; i < argv.length; i++) {
    const arg = argv[i];
    const next = () => argv[i + 1];
    switch (arg) {
      case "--build-dir":
        args.buildDir = next() ?? args.buildDir;
        i += 1;
        break;
      case "--no-build":
        args.noBuild = true;
        break;
      case "--verbose":
      case "-v":
        args.verbose = true;
        break;
      case "--tracing":
        args.tracing = true;
        break;
      default:
        break;
    }
  }
  return args;
}

function runScript(
  scriptPath: string,
  extraArgs: string[]
): Promise<{ code: number | null }> {
  return new Promise((resolve) => {
    const child = spawn(process.execPath, ["--import", "tsx", scriptPath, ...extraArgs], {
      stdio: "inherit",
    });
    child.on("close", (code) => resolve({ code: code ?? null }));
  });
}

async function getMtimeMs(filePath: string): Promise<number | null> {
  try {
    const info = await stat(filePath);
    return info.mtimeMs;
  } catch {
    return null;
  }
}

async function main() {
  const argv = parseArgs(process.argv.slice(2));

  const scriptDir = path.dirname(new URL(import.meta.url).pathname);
  const projectRoot = path.resolve(scriptDir, "..");
  const buildDir = path.resolve(projectRoot, argv.buildDir);

  const testsDir = path.join(buildDir, "tests");
  const unitOutputDir = path.join(testsDir, "unit_output");
  const renderOutputDir = path.join(buildDir, "tests", "render_output");

  const unitArgs = [
    "--build-dir",
    buildDir,
    "--output-dir",
    unitOutputDir,
    ...(argv.noBuild ? ["--no-build"] : []),
    ...(argv.verbose ? ["--verbose"] : []),
  ];

  // Tracing is opt-in: pass --tracing to enable; otherwise pass --skip-trace (default fast).
  const renderArgs = [
    "--build-dir",
    buildDir,
    "--output-dir",
    renderOutputDir,
    ...(argv.noBuild ? ["--no-build"] : []),
    ...(argv.verbose ? ["--verbose"] : []),
    ...(argv.tracing ? ["--tracing"] : ["--skip-trace"]),
  ];

  const mainReportPath = path.join(testsDir, "tests_report.html");

  console.log("\n\u001b[35m\u001b[1mRun all tests (unit + render)\u001b[22m\u001b[39m\n");
  console.log(`Build directory: ${buildDir}`);
  console.log(`Main report:     ${mainReportPath}`);
  console.log();

  const unitPath = path.join(scriptDir, "run_unit_tests.ts");
  const unitResultsJson = path.join(unitOutputDir, "unit_results.json");
  const unitResultsBefore = await getMtimeMs(unitResultsJson);
  const unitRes = await runScript(unitPath, unitArgs);

  const renderPath = path.join(scriptDir, "render", "run_render_tests.ts");
  const renderResultsJson = path.join(renderOutputDir, "render_results.json");
  const renderResultsBefore = await getMtimeMs(renderResultsJson);
  const renderRes = await runScript(renderPath, renderArgs);

  const unitResultsAfter = await getMtimeMs(unitResultsJson);
  const renderResultsAfter = await getMtimeMs(renderResultsJson);
  const unitResultsUpdated =
    unitResultsAfter !== null && unitResultsAfter !== unitResultsBefore;
  const renderResultsUpdated =
    renderResultsAfter !== null && renderResultsAfter !== renderResultsBefore;

  const unitOk = unitRes.code === 0 && unitResultsUpdated;
  const renderOk = renderRes.code === 0 && renderResultsUpdated;

  if (!unitOk || !renderOk) {
    process.exitCode = 1;
  }

  // Generate combined report
  const reportToolsDir = path.join(scriptDir, "report-tools-ts");
  let genRes: { code: number | null } | null = null;

  if (unitResultsUpdated && renderResultsUpdated) {
    console.log("\n\u001b[35m\u001b[1mGenerate combined report\u001b[22m\u001b[39m");

    genRes = await runScript(
      path.join(reportToolsDir, "scripts", "generate.ts"),
      [
        "--mode", "combined",
        "--output-dir", testsDir,
        "--unit-results", unitResultsJson,
        "--render-results", renderResultsJson,
      ]
    );
  } else {
    console.log("\n\u001b[33m\u001b[1mSkip combined report\u001b[22m\u001b[39m");
    if (!unitResultsUpdated) {
      console.log("  Unit results were not updated in this run.");
    }
    if (!renderResultsUpdated) {
      console.log("  Render results were not updated in this run.");
    }
  }

  if (genRes && genRes.code !== 0) {
    console.log("\u001b[31m\u001b[1mFailed to generate combined report\u001b[22m\u001b[39m");
    process.exitCode = 1;
  }

  const allPassed = unitOk && renderOk && (!genRes || genRes.code === 0);
  const statusColor = allPassed ? "\u001b[32m" : "\u001b[31m";
  const statusText = allPassed ? "All tests completed successfully." : "Some tests failed.";

  console.log(`\n${statusColor}\u001b[1m${statusText}\u001b[22m\u001b[39m`);
  console.log(`  Unit:   ${unitOk ? "PASSED" : "FAILED"} (tests_report.html - Unit tab)`);
  console.log(`  Render: ${renderOk ? "PASSED" : "FAILED"} (tests_report.html - Render tab)`);
  console.log(`  Main:   ${mainReportPath}`);
}

main().catch((err) => {
  console.error(err);
  process.exit(1);
});
