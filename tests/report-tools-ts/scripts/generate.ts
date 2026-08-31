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

import {readFile, writeFile, mkdir} from 'fs/promises';
import {basename, join} from 'path';
import {
  buildTree,
  pruneStdNodes,
  annotateStats,
} from '../src/algorithms/treeBuilder';
import {generateReportHtml, generateTraceHtml} from '../src/utils/renderToHtml';
import type {TraceEvent, TestReportData, CliArgs} from '../src/types';

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
 */
async function transformResults(
  rawData: unknown[],
  outputDir: string
): Promise<TestReportData[]> {
  return Promise.all(
    rawData.map(async (item: any) => {
      // Build expected trace filename pattern
      // Format: trace__{Suite}__{TestName}.json/.html
      const tracePattern = `trace__${item.test_suite}__${item.test_name}`;
      const expectedTraceJson = join(outputDir, `${tracePattern}.json`);
      const expectedTraceHtml = join(outputDir, `${tracePattern}.html`);

      // Check if trace JSON file exists (HTML will be generated from it)
      let traceHtmlFilename: string | null = null;
      try {
        await readFile(expectedTraceJson);
        // JSON exists, so HTML should be generated
        traceHtmlFilename = expectedTraceHtml;
      } catch {
        // Check if it's already in the traces array
        if (item.traces && item.traces.length > 0) {
          const matchingTrace = item.traces.find((t: string) =>
            t.includes(tracePattern)
          );
          if (matchingTrace) {
            traceHtmlFilename = matchingTrace.replace('.json', '.html');
          }
        }
      }

      // Read MD5 hash from the first .md5 file if available
      let md5Hash: string | null = null;
      if (item.md5s && item.md5s.length > 0) {
        md5Hash = await readMd5Hash(item.md5s[0]);
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
        pngFilename: item.pngs && item.pngs.length > 0 ? item.pngs[0] : null,
        traceHtmlFilename,
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

    console.log('\n✓ Generation complete!');
  } catch (error) {
    console.error('Error:', error);
    process.exit(1);
  }
}

main();
