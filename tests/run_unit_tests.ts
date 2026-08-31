#!/usr/bin/env tsx
/**
 * Unit test runner: runs all non-render GTest executables and writes unit_results.json.
 * HTML report generation is now handled by the combined report in run_tests.ts.
 *
 * Usage:
 *   npx tsx tests/run_unit_tests.ts [--build-dir build] [--output-dir build/tests] [--no-build] [--verbose]
 */

import path from "node:path";
import fs from "node:fs/promises";
import { existsSync } from "node:fs";
import { spawn } from "node:child_process";
import ora from "ora";
import chalk from "chalk";

const purple = chalk.hex("#bb00ff");
const green = chalk.green.bold;
const red = chalk.red.bold;
const dim = chalk.dim;

/** Non-render test executables (from tests/CMakeLists.txt gtest_discover_tests). */
const UNIT_TEST_EXECUTABLES = [
  "physics_tests",
  "cfile_mock_tests",
  "cfile_coverage_tests",
  "bitmap_loader_tests",
  "iff_loader_tests",
  "pcx_loader_tests",
  "manage_tests",
  "weapon_external_tests",
  "damage_external_tests",
  "levelgoal_external_tests",
  "room_external_tests",
  "object_external_tests",
  "object_constants_tests",
  "player_controls_constants_tests",
  "ship_constants_tests",
  "weapon_constants_tests",
  "ai_constants_tests",
  "world_constants_tests",
  "gamestate_constants_tests",
  "pilot_config_constants_tests",
  "othersystems_constants_tests",
  "telcom_constants_tests",
  "vfx_constants_tests",
] as const;

type UnitResult = {
  exe: string;
  test_suite: string;
  test_name: string;
  passed: boolean;
  duration_ms: number;
};

type Args = {
  buildDir: string;
  outputDir: string;
  noBuild: boolean;
  verbose: boolean;
};

function parseArgs(argv: string[]): Args {
  const args: Args = {
    buildDir: "build",
    outputDir: "",
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
      case "--output-dir":
        args.outputDir = next() ?? "";
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

function parseGTestOutput(output: string, exe: string): UnitResult[] {
  const results: UnitResult[] = [];
  const okPattern = /\[\s+OK\s+]\s+(\S+)\.(\S+)\s+\((\d+)\s*ms\)/g;
  const failedPattern = /\[\s+FAILED\s+]\s+(\S+)\.(\S+)\s+\((\d+)\s*ms\)/g;

  let m: RegExpExecArray | null;
  while ((m = okPattern.exec(output))) {
    results.push({
      exe,
      test_suite: m[1],
      test_name: m[2],
      passed: true,
      duration_ms: parseInt(m[3], 10),
    });
  }
  while ((m = failedPattern.exec(output))) {
    results.push({
      exe,
      test_suite: m[1],
      test_name: m[2],
      passed: false,
      duration_ms: parseInt(m[3], 10),
    });
  }
  return results;
}

async function runCommand(
  cmd: string,
  args: string[],
  options: { cwd?: string; verbose?: boolean } = {}
): Promise<{ code: number | null; stdout: string; stderr: string }> {
  const { cwd, verbose } = options;

  if (verbose) {
    console.log(dim(`   $ ${cmd} ${args.join(" ")}`));
    if (cwd) console.log(dim(`   cwd=${cwd}`));
  }

  return new Promise((resolve, reject) => {
    const child = spawn(cmd, args, {
      cwd,
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

    child.on("error", reject);
    child.on("close", (code) => {
      resolve({ code, stdout, stderr });
    });
  });
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
    ["-DBUILD_TESTING=ON", projectRoot],
    { cwd: buildDir, verbose }
  );
  return res.code === 0;
}

async function ensureBuilt(
  buildDir: string,
  target: string,
  verbose: boolean
): Promise<boolean> {
  const res = await runCommand("cmake", ["--build", buildDir, "--target", target], {
    cwd: buildDir,
    verbose,
  });
  return res.code === 0;
}

async function main() {
  const argv = parseArgs(process.argv.slice(2));

  const scriptDir = path.dirname(new URL(import.meta.url).pathname);
  const testsDir = path.resolve(scriptDir);
  const projectRoot = path.resolve(testsDir, "..");

  const buildDir = path.resolve(projectRoot, argv.buildDir);
  const outputDir = argv.outputDir
    ? path.resolve(argv.outputDir)
    : path.join(buildDir, "tests", "unit_output");

  console.log("\n" + purple.bold("Unit Test Runner\n"));
  console.log(dim("Build directory:  ") + buildDir);
  console.log(dim("Output directory: ") + outputDir);
  console.log();

  await fs.mkdir(outputDir, { recursive: true });

  if (!argv.noBuild) {
    const spinner = ora({
      text: purple("Configure CMake"),
      color: "magenta",
      spinner: "dots",
    }).start();
    const configured = await ensureCmakeConfigured(buildDir, projectRoot, argv.verbose);
    if (!configured) {
      spinner.fail(red("Configure CMake"));
      process.exit(1);
    }
    spinner.succeed(green("Configure CMake"));

    const buildSpinner = ora({
      text: purple("Build unit test binaries"),
      color: "magenta",
      spinner: "dots",
    }).start();
    for (const exe of UNIT_TEST_EXECUTABLES) {
      const ok = await ensureBuilt(buildDir, exe, argv.verbose);
      if (!ok) {
        buildSpinner.fail(red(`Build failed for ${exe}`));
        process.exit(1);
      }
    }
    buildSpinner.succeed(green("Build unit test binaries"));
  }

  const allResults: UnitResult[] = [];
  let totalPassed = 0;
  let totalFailed = 0;

  const runSpinner = ora({
    text: purple("Run unit tests"),
    color: "magenta",
    spinner: "dots",
  }).start();

  const testsBinDir = path.join(buildDir, "tests");

  for (const exe of UNIT_TEST_EXECUTABLES) {
    const binaryPath = path.join(testsBinDir, exe);
    if (!existsSync(binaryPath)) {
      if (argv.verbose) {
        runSpinner.info(dim(`   Skip ${exe} (not built)`));
      }
      continue;
    }

    const res = await runCommand(binaryPath, [], {
      cwd: testsBinDir,
      verbose: argv.verbose,
    });

    const combined = res.stdout + res.stderr;
    const parsed = parseGTestOutput(combined, exe);

    for (const r of parsed) {
      allResults.push(r);
      if (r.passed) totalPassed += 1;
      else totalFailed += 1;
    }

    if (parsed.length === 0 && res.code !== 0 && !argv.verbose) {
      runSpinner.warn(dim(`   ${exe} exited with code ${res.code} (no parsed tests)`));
    }
  }

  runSpinner.succeed(green("Run unit tests"));

  const resultsPath = path.join(outputDir, "unit_results.json");
  await fs.writeFile(resultsPath, JSON.stringify(allResults, null, 2), "utf8");
  console.log(dim(`   Results: ${resultsPath}`));

  const total = totalPassed + totalFailed;
  const summaryWidth = 50;
  const pad = (s: string) => s.padEnd(summaryWidth).slice(0, summaryWidth);
  const border = "═".repeat(summaryWidth);
  console.log();
  console.log(green.bold("╔" + border + "╗"));
  console.log(
    green.bold("║") +
      dim(pad(`Total: ${total}  Passed: ${totalPassed}  Failed: ${totalFailed}`)) +
      green.bold("║")
  );
  console.log(green.bold("║") + dim(pad("Results: " + resultsPath)) + green.bold("║"));
  console.log(green.bold("╚" + border + "╝"));
  console.log();

  process.exitCode = totalFailed === 0 ? 0 : 1;
}

main().catch((err) => {
  console.error(red("Unexpected error:"), err);
  process.exit(1);
});
