#!/usr/bin/env tsx
/**
 * Main CLI script for generating HTML reports.
 *
 * Usage:
 *   tsx scripts/generate.ts --mode report --output-dir ./out --results ./results.json
 *   tsx scripts/generate.ts --mode trace --output-dir ./out --traces trace1.json trace2.json
 *
 * @module scripts/generate
 */

import {readFile, writeFile, mkdir, readdir} from 'fs/promises';
import {basename, join, resolve} from 'path';
import {findMatchingPng} from '../src/utils/fileHelpers';
import {
  buildTree,
  pruneStdNodes,
  annotateStats,
} from '../src/algorithms/treeBuilder';
import {
  generateReportHtml,
  generateTraceHtml,
  generateUnitReportHtml,
  generateCombinedReportHtml,
} from '../src/utils/renderToHtml';
import type {
  TraceEvent,
  TestReportData,
  UnitTestReportData,
  CliArgs,
} from '../src/types';

/**
 * Parses command line arguments.
 *
 * @param args - Process arguments
 * @returns Parsed CLI arguments
 */
function parseArgs(args: string[]): CliArgs {
  const result: CliArgs = {
    mode: 'all',
    outputDir: './output',
  };

  for (let i = 0; i < args.length; i++) {
    const arg = args[i];

    switch (arg) {
      case '--mode':
        result.mode = args[++i] as CliArgs['mode'];
        if (
          result.mode !== 'report' &&
          result.mode !== 'trace' &&
          result.mode !== 'unit-report' &&
          result.mode !== 'combined' &&
          result.mode !== 'all'
        ) {
          console.error(`Error: unknown mode "${result.mode}"`);
          process.exit(1);
        }
        break;
      case '--output-dir':
        result.outputDir = args[++i];
        break;
      case '--results':
        result.resultsJson = args[++i];
        break;
      case '--traces':
        result.traceFiles = [];
        // Collect all trace files until next flag
        let j = i + 1;
        while (j < args.length && !args[j].startsWith('--')) {
          result.traceFiles.push(args[j]);
          j++;
        }
        i = j - 1;
        break;
      case '--unit-results':
        result.unitResultsJson = args[++i];
        break;
      case '--render-results':
        result.renderResultsJson = args[++i];
        break;
    }
  }

  return result;
}

/**
 * Reads MD5 hash from a file, returns null if file doesn't exist or is empty.
 */
async function readMd5Hash(md5Path: string): Promise<string | null> {
  try {
    const content = await readFile(md5Path, 'utf-8');
    return content.trim() || null;
  } catch {
    return null;
  }
}

/**
 * Transforms raw JSON data from runner format to TestReportData format.
 * Converts snake_case field names to camelCase.
 * When the runner didn't attach a PNG path, tries to find one in outputDir by pattern Test{Suite}_{Test}_*.png.
 */
async function transformResults(
  rawData: unknown[],
  outputDir: string
): Promise<TestReportData[]> {
  // List PNGs in outputDir once for fallback when item.pngs is empty
  let outputPngPaths: string[] = [];
  try {
    const names = await readdir(outputDir);
    outputPngPaths = names
      .filter((f) => f.startsWith('Test') && f.endsWith('.png'))
      .map((f) => join(outputDir, f));
  } catch {
    // outputDir may not exist
  }

  return Promise.all(
    rawData.map(async (item: any) => {
      // Build expected trace filename pattern
      // Format: trace__{Suite}__{TestName}.json
      const tracePattern = `trace__${item.test_suite}__${item.test_name}`;
      const expectedTraceJson = join(outputDir, `${tracePattern}.json`);

      // Check if trace JSON file exists and read its data
      let traceData: object | null = null;
      try {
        const traceJson = await readFile(expectedTraceJson, 'utf-8');
        traceData = JSON.parse(traceJson);
      } catch {
        // Check if it's already in the traces array
        if (item.traces && item.traces.length > 0) {
          const matchingTrace = item.traces.find((t: string) =>
            t.includes(tracePattern)
          );
          if (matchingTrace) {
            try {
              const traceJson = await readFile(matchingTrace, 'utf-8');
              traceData = JSON.parse(traceJson);
            } catch {
              // Could not read trace file
            }
          }
        }
      }

      // Read MD5 hash from the first .md5 file if available
      let md5Hash: string | null = null;
      if (item.md5s && item.md5s.length > 0) {
        md5Hash = await readMd5Hash(item.md5s[0]);
      }

      // PNG: resolve from outputDir first (so we use files that exist on disk), then fall back to JSON
      let pngFilename: string | null = null;
      if (outputPngPaths.length > 0) {
        pngFilename = findMatchingPng(
          item.test_suite || '',
          item.test_name || '',
          outputPngPaths
        );
      }
      if (!pngFilename && item.pngs && item.pngs.length > 0) {
        pngFilename = basename(item.pngs[0]);
      }

      return {
        testName: item.test_name || '',
        testSuite: item.test_suite || '',
        executable: item.exe || '',
        durationMs: item.duration_ms || 0,
        md5Hash,
        previousMd5: null, // Will be computed later if needed
        status: item.passed ? 'PASSED' : 'FAILED',
        outputText: '',
        environment: item.environment || {},
        renderFunctions: [],
        pngFilename,
        traceData,
        callgraphs: item.callgraphs || [],
        passed: item.passed || false,
      };
    })
  );
}

/**
 * Generates the main report HTML.
 *
 * @param args - CLI arguments
 */
async function generateReport(args: CliArgs): Promise<void> {
  if (!args.resultsJson) {
    console.error('Error: --results required for report mode');
    process.exit(1);
  }

  console.log('Reading results...');
  const resultsData = await readFile(args.resultsJson, 'utf-8');
  const rawResults = JSON.parse(resultsData);
  const results = await transformResults(rawResults, args.outputDir);

  console.log(`Generating report for ${results.length} tests...`);

  const html = generateReportHtml({
    generatedAt: new Date().toISOString(),
    tracingEnabled: true,
    total: results.length,
    passed: results.filter((r) => r.passed).length,
    failed: results.filter((r) => !r.passed).length,
    md5Regressions: 0, // Calculate from results if needed
    results,
    updateBaseline: process.env.UPDATE_BASELINE === '1',
  });

  const outputPath = join(args.outputDir, 'render_report.html');
  await mkdir(args.outputDir, {recursive: true});
  await writeFile(outputPath, html);
  console.log(`✓ Generated ${outputPath}`);
}

/**
 * Transforms raw unit_results.json (snake_case) to UnitTestReportData[].
 */
function transformUnitResults(rawData: unknown[]): UnitTestReportData[] {
  return rawData.map((item: any) => ({
    executable: item.exe || '',
    testSuite: item.test_suite || '',
    testName: item.test_name || '',
    durationMs: item.duration_ms ?? 0,
    status: item.passed ? 'PASSED' : 'FAILED',
    passed: Boolean(item.passed),
  }));
}

/**
 * Generates the unit test report HTML.
 *
 * @param args - CLI arguments
 */
async function generateUnitReport(args: CliArgs): Promise<void> {
  if (!args.resultsJson) {
    console.error('Error: --results required for unit-report mode');
    process.exit(1);
  }

  console.log('Reading unit results...');
  const resultsData = await readFile(args.resultsJson, 'utf-8');
  const rawResults = JSON.parse(resultsData);
  const results = transformUnitResults(Array.isArray(rawResults) ? rawResults : []);

  console.log(`Generating unit report for ${results.length} tests...`);

  const passed = results.filter((r) => r.passed).length;
  const failed = results.length - passed;

  const html = generateUnitReportHtml({
    generatedAt: new Date().toISOString(),
    total: results.length,
    passed,
    failed,
    results,
  });

  const outputPath = join(args.outputDir, 'unit_report.html');
  await mkdir(args.outputDir, {recursive: true});
  await writeFile(outputPath, html);
  console.log(`✓ Generated ${outputPath}`);
}

/**
 * Generates the combined test report HTML with Unit and Render tabs.
 *
 * @param args - CLI arguments
 */
async function generateCombinedReport(args: CliArgs): Promise<void> {
  if (!args.unitResultsJson || !args.renderResultsJson) {
    console.error('Error: --unit-results and --render-results required for combined mode');
    process.exit(1);
  }

  console.log('Reading unit results...');
  const unitResultsData = await readFile(args.unitResultsJson, 'utf-8');
  const rawUnitResults = JSON.parse(unitResultsData);
  const unitResults = transformUnitResults(Array.isArray(rawUnitResults) ? rawUnitResults : []);

  console.log('Reading render results...');
  const renderResultsData = await readFile(args.renderResultsJson, 'utf-8');
  const rawRenderResults = JSON.parse(renderResultsData);
  // Resolve to absolute path so PNG lookup works regardless of script cwd
  const renderOutputDir = resolve(args.outputDir, 'render_output');
  const renderResults = await transformResults(rawRenderResults, renderOutputDir);

  const unitPassed = unitResults.filter((r) => r.passed).length;
  const unitFailed = unitResults.length - unitPassed;

  const renderPassed = renderResults.filter((r) => r.passed).length;
  const renderFailed = renderResults.length - renderPassed;

  // Count MD5 regressions
  let md5Regressions = 0;
  for (const test of renderResults) {
    if (test.md5Hash && test.previousMd5 && test.md5Hash !== test.previousMd5) {
      md5Regressions++;
    }
  }

  console.log(`Generating combined report for ${unitResults.length} unit + ${renderResults.length} render tests...`);

  const html = generateCombinedReportHtml({
    generatedAt: new Date().toISOString(),
    unitTotal: unitResults.length,
    unitPassed,
    unitFailed,
    unitResults,
    renderTotal: renderResults.length,
    renderPassed,
    renderFailed,
    renderResults,
    tracingEnabled: renderResults.some((r) => r.traceData !== null),
    md5Regressions,
  });

  const outputPath = join(args.outputDir, 'tests_report.html');
  await mkdir(args.outputDir, {recursive: true});
  await writeFile(outputPath, html);
  console.log(`✓ Generated ${outputPath}`);
}

/**
 * Generates per-trace HTML files.
 *
 * @param args - CLI arguments
 */
async function generateTraces(args: CliArgs): Promise<void> {
  if (!args.traceFiles || args.traceFiles.length === 0) {
    console.error('Error: --traces required for trace mode');
    process.exit(1);
  }

  for (const traceFile of args.traceFiles) {
    console.log(`Processing ${basename(traceFile)}...`);

    const traceData = await readFile(traceFile, 'utf-8');
    const events: TraceEvent[] = JSON.parse(traceData);

    // Build and process tree
    let tree = buildTree(events);
    pruneStdNodes(tree);
    annotateStats(tree);

    // Extract test name from filename
    const testName = basename(traceFile, '.json')
      .replace('trace__', '')
      .replace(/__/g, '.');

    // Generate HTML
    const html = generateTraceHtml(JSON.stringify(tree), testName);

    const outputFile = join(
      args.outputDir,
      basename(traceFile, '.json') + '.html'
    );
    await writeFile(outputFile, html);
    console.log(`✓ Generated ${basename(outputFile)}`);
  }
}

/**
 * Main entry point.
 */
async function main(): Promise<void> {
  const args = parseArgs(process.argv.slice(2));

  console.log('Report Generation Tool (TypeScript)');
  console.log(`Mode: ${args.mode}`);
  console.log(`Output: ${args.outputDir}`);
  console.log('');

  try {
    if (args.mode === 'report' || args.mode === 'all') {
      await generateReport(args);
    }

    if (args.mode === 'trace' || args.mode === 'all') {
      await generateTraces(args);
    }

    if (args.mode === 'unit-report') {
      await generateUnitReport(args);
    }

    if (args.mode === 'combined') {
      await generateCombinedReport(args);
    }

    console.log('\n✓ Generation complete!');
  } catch (error) {
    console.error('Error:', error);
    process.exit(1);
  }
}

main();
