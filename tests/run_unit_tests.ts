#!/usr/bin/env tsx
/**
 * Unit test runner: runs all non-render GTest executables and writes unit_results.json.
 * It also republishes a compatibility copy under build/tests/ and generates unit_report.html.
 *
 * Usage:
 *   npx tsx tests/run_unit_tests.ts [--build-dir build] [--output-dir build/tests/unit_output] [--no-build] [--verbose]
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
  "vecmat_util_tests",
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
  "args_tests_standalone",
  "byteswap_tests",
  "byteswap_tests_standalone",
  "fix_tests_standalone",
  "macros_tests_standalone",
  "md5_tests_standalone",
  "misc_tests",
  "psclass_tests_standalone",
  "psglob_tests_standalone",
  "psrand_tests_standalone",
  "pstring_tests_standalone",
  "vecmat_tests_standalone",
  "vecmat_external_tests_standalone",
  "md5_tests",
  "ddio_tests",
  "misc_util_tests",
  "psclass_real_tests",
  "fix_real_tests",
  "psrand_real_tests",
  "mem_real_tests",
  "lib_util_tests",
  "ddio_file_tests",
  "unzip_real_tests",
  "list_real_tests",
  "small_game_real_tests",
  "door_ship_vclip_real_tests",
  "aiambient_real_tests",
  "aiterrain_real_tests",
  "trigger_real_tests",
  "marker_real_tests",
  "gamespyutils_real_tests",
  "args_real_tests",
  "objinfo_real_tests",
  "program_real_tests",
  "splinter_real_tests",
  "soar_real_tests",
  "bnode_real_tests",
  "gameevent_real_tests",
  "postrender_real_tests",
  "subtitles_real_tests",
  "weather_real_tests",
  "voice_real_tests",
  "credits_real_tests",
  "scorch_real_tests",
  "robotfire_real_tests",
  "vclip_real_tests",
  "spew_real_tests",
  "lightmap_info_real_tests",
  "gamepath_gamefile_real_tests",
  "ambient_real_tests",
  "objscript_real_tests",
  "special_face_real_tests",
  "multi_save_setting_real_tests",
  "briefingparse_real_tests",
  "inventory_real_tests",
  "hudmessage_real_tests",
  "pilot_class_real_tests",
  "localization_real_tests",
  "attach_real_tests",
  "doorway_real_tests",
  "object_lighting_real_tests",
  "audiotaunts_real_tests",
  "pilotpics_real_tests",
  "gamefont_real_tests",
  "cinematics_real_tests",
  "megacell_real_tests",
  "debuggraph_real_tests",
  "d3music_real_tests",
  "d3movie_real_tests",
  "newpyrogauges_real_tests",
  "gametexture_real_tests",
  "telcomcargo_real_tests",
  "help_real_tests",
  "dllwrappers_real_tests",
  "sdlmain_real_tests",
  "buddymenu_real_tests",
  "newui_filedlg_real_tests",
  "slew_real_tests",
  "mmitem_real_tests",
  "smallviews_real_tests",
  "cockpit_real_tests",
  "d3forcefeedback_real_tests",
  "gamecheat_real_tests",
  "screens_real_tests",
  "multiclient_real_tests",
  "telcomgoals_real_tests",
  "game2dll_real_tests",
  "telcomautomap_real_tests",
  "terrainsearch_real_tests",
  "multiconnect_real_tests",
  "hotspotmap_real_tests",
  "mission_real_tests",
  "object_real_tests",
  "matcen_real_tests",
  "loadstate_real_tests",
  "viseffect_real_tests",
  "gamecinematics_real_tests",
  "gamesequence_real_tests",
  "fireball_real_tests",
  "boa_real_tests",
  "multisafe_real_tests",
  "multiserver_real_tests",
  "gameloop_real_tests",
  "player_real_tests",
  "loadlevel_real_tests",
  "newuicore_real_tests",
  "config_real_tests",
  "menu_real_tests",
  "game_real_tests",
  "bnode_linked_real_tests",
  "boa_linked_real_tests",
  "aigoal_linked_real_tests",
  "lighting_linked_real_tests",
  "terrainrender_linked_real_tests",
  "damage_linked_real_tests",
  "object_linked_real_tests",
  "aimain_linked_real_tests",
  "room_linked_real_tests",
  "bsp_linked_real_tests",
  "terrain_linked_real_tests",
  "help_linked_real_tests",
  "door_linked_real_tests",
  "scorch_linked_real_tests",
  "configitem_linked_real_tests",
  "gamecheat_linked_real_tests",
   "fireball_linked_real_tests",
    "weaponfire_linked_real_tests",
    "player_linked_real_tests",
    "loadlevel_linked_real_tests",
    "telcom_linked_real_tests",
    "multi_linked_real_tests",
    "mission_linked_real_tests",
    "pilot_linked_real_tests",
    "newui_linked_real_tests",
    "osiris_linked_real_tests",
    "matcen_linked_real_tests",
    "gamesequence_linked_real_tests",
     "osiris_predefs_linked_real_tests",
     "loadstate_linked_real_tests",
     "gameloop_linked_real_tests",
      "multi_server_linked_real_tests",
      "ctlconfig_linked_real_tests",
      "controls_linked_real_tests",
      "gauges_linked_real_tests",
      "multi_client_linked_real_tests",
      "multi_connect_linked_real_tests",
      "ctlcfgelem_linked_real_tests",
      "multi_dll_mgr_linked_real_tests",
      "mission_download_linked_real_tests",
      "multisafe_server_linked_real_tests",
      "osirisloadandbind_linked_real_tests",
      "soar_helpers_linked_real_tests",
    "telcom_real_tests",
  "osirisload_real_tests",
  "aimain_real_tests",
  "weaponfire_real_tests",
  "pilot_real_tests",
  "multi_real_tests",
  "terrainrender_real_tests",
  "osiris_predefs_real_tests",
  "room_real_tests",
  "damage_real_tests",
  "levelgoal_real_tests",
  "bsp_real_tests",
  "terrain_real_tests",
  "aigoal_real_tests",
  "gamesave_real_tests",
  "procedurals_real_tests",
  "hud_real_tests",
  "telcomeffects_real_tests",
  "objectlighting_real_tests",
  "aipath_real_tests",
  "objinit_real_tests",
  "telcomeffectsrender_real_tests",
  "huddisplay_real_tests",
  "descent_real_tests",
  "renderobject_real_tests",
  "dedicated_server_real_tests",
  "demofile_real_tests",
   "init_real_tests",
   "multi_ui_real_tests",
   "gamefile_real_tests",
   "gamepath_real_tests",
   "door_real_tests",
   "ship_real_tests",
   "pilotpicsapi_real_tests",
   "weapon_real_tests",
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
      stdio: ["ignore", "pipe", "pipe"],
    });

    let stdout = "";
    let stderr = "";

    if (child.stdout) {
      child.stdout.on("data", (d) => {
        const chunk = d.toString();
        stdout += chunk;
        if (verbose) {
          process.stdout.write(chunk);
        }
      });
    }
    if (child.stderr) {
      child.stderr.on("data", (d) => {
        const chunk = d.toString();
        stderr += chunk;
        if (verbose) {
          process.stderr.write(chunk);
        }
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

async function generateUnitReport(
  projectRoot: string,
  testsDir: string,
  resultsPath: string,
  verbose: boolean
): Promise<boolean> {
  const reportScript = path.join(
    projectRoot,
    "tests",
    "report-tools-ts",
    "scripts",
    "generate.ts"
  );
  const res = await runCommand(
    process.execPath,
    ["--import", "tsx", reportScript, "--mode", "unit-report", "--results", resultsPath, "--output-dir", testsDir],
    { cwd: projectRoot, verbose }
  );
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
  const zeroParsedExecutables: string[] = [];

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

    if (parsed.length === 0) {
      zeroParsedExecutables.push(exe);
      if (!argv.verbose) {
        const suffix = res.code === 0 ? "exited successfully" : `exited with code ${res.code}`;
        runSpinner.warn(dim(`   ${exe} ${suffix} (no parsed tests)`));
      }
    }
  }

  runSpinner.succeed(green("Run unit tests"));

  const resultsPath = path.join(outputDir, "unit_results.json");
  const serializedResults = JSON.stringify(allResults, null, 2);
  await fs.writeFile(resultsPath, serializedResults, "utf8");
  console.log(dim(`   Results: ${resultsPath}`));

  const legacyResultsPath = path.join(testsBinDir, "unit_results.json");
  if (legacyResultsPath !== resultsPath) {
    await fs.writeFile(legacyResultsPath, serializedResults, "utf8");
    console.log(dim(`   Compatibility results: ${legacyResultsPath}`));
  }

  const unitReportOk = await generateUnitReport(
    projectRoot,
    testsBinDir,
    legacyResultsPath,
    argv.verbose
  );
  if (unitReportOk) {
    console.log(dim(`   Unit report: ${path.join(testsBinDir, "unit_report.html")}`));
  } else {
    console.log(dim("   Warning: failed to generate unit_report.html"));
  }

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

  if (zeroParsedExecutables.length > 0) {
    console.log(
      dim(
        `Unparsed executables: ${zeroParsedExecutables.join(", ")}`
      )
    );
  }

  const noResultsParsed = allResults.length === 0 && zeroParsedExecutables.length > 0;
  process.exitCode = totalFailed === 0 && unitReportOk && !noResultsParsed ? 0 : 1;
}

main().catch((err) => {
  console.error(red("Unexpected error:"), err);
  process.exit(1);
});
