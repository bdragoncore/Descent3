#!/usr/bin/env tsx
/**
 * Run all tests: unit tests first, then render tests.
 * Uses the same --build-dir (and optionally --no-build, --verbose) for both.
 *
 * Usage:
 *   npm run tests [-- --no-build] [-- --verbose]
 *
 * Output:
 *   build-dir/tests/tests_report.html  — combined report with Unit/Render tabs
 *   build-dir/tests/render_output/     — PNGs, traces, etc.
 */

import path from "node:path";
import { spawn } from "node:child_process";

function parseArgs(argv: string[]): { buildDir: string; noBuild: boolean; verbose: boolean } {
  const args = {
    buildDir: "build",
    noBuild: false,
    verbose: false,
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
    const child = spawn("npx", ["tsx", scriptPath, ...extraArgs], {
      stdio: "inherit",
      shell: true,
    });
    child.on("close", (code) => resolve({ code: code ?? null }));
  });
}

async function main() {
  const argv = parseArgs(process.argv.slice(2));

  const scriptDir = path.dirname(new URL(import.meta.url).pathname);
  const projectRoot = path.resolve(scriptDir, "..");
  const buildDir = path.resolve(projectRoot, argv.buildDir);

  const testsDir = path.join(buildDir, "tests");
  const renderOutputDir = path.join(buildDir, "tests", "render_output");

  const unitArgs = [
    "--build-dir",
    buildDir,
    "--output-dir",
    testsDir,
    ...(argv.noBuild ? ["--no-build"] : []),
    ...(argv.verbose ? ["--verbose"] : []),
  ];

  const renderArgs = [
    "--build-dir",
    buildDir,
    "--output-dir",
    renderOutputDir,
    ...(argv.noBuild ? ["--no-build"] : []),
    ...(argv.verbose ? ["--verbose"] : []),
  ];

  const mainReportPath = path.join(testsDir, "tests_report.html");

  console.log("\n\u001b[35m\u001b[1mRun all tests (unit + render)\u001b[22m\u001b[39m\n");
  console.log(`Build directory: ${buildDir}`);
  console.log(`Main report:     ${mainReportPath}`);
  console.log();

  const unitPath = path.join(scriptDir, "run_unit_tests.ts");
  const unitRes = await runScript(unitPath, unitArgs);

  const renderPath = path.join(scriptDir, "render", "run_render_tests.ts");
  const renderRes = await runScript(renderPath, renderArgs);

  if (unitRes.code !== 0 || renderRes.code !== 0) {
    process.exitCode = 1;
  }

  // Generate combined report
  const reportToolsDir = path.join(scriptDir, "report-tools-ts");
  const unitResultsJson = path.join(testsDir, "unit_results.json");
  const renderResultsJson = path.join(renderOutputDir, "render_results.json");
  
  console.log("\n\u001b[35m\u001b[1mGenerate combined report\u001b[22m\u001b[39m");
  
  const genRes = await runScript(
    path.join(reportToolsDir, "scripts", "generate.ts"),
    [
      "--mode", "combined",
      "--output-dir", testsDir,
      "--unit-results", unitResultsJson,
      "--render-results", renderResultsJson,
    ]
  );
  
  if (genRes.code !== 0) {
    console.log("\u001b[31m\u001b[1mFailed to generate combined report\u001b[22m\u001b[39m");
    process.exitCode = 1;
  }

  const allPassed = unitRes.code === 0 && renderRes.code === 0;
  const statusColor = allPassed ? "\u001b[32m" : "\u001b[31m";
  const statusText = allPassed ? "All tests completed successfully." : "Some tests failed.";

  console.log(`\n${statusColor}\u001b[1m${statusText}\u001b[22m\u001b[39m`);
  console.log(`  Unit:   ${unitRes.code === 0 ? "PASSED" : "FAILED"} (tests_report.html - Unit tab)`);
  console.log(`  Render: ${renderRes.code === 0 ? "PASSED" : "FAILED"} (tests_report.html - Render tab)`);
  console.log(`  Main:   ${mainReportPath}`);
}

main().catch((err) => {
  console.error(err);
  process.exit(1);
});
