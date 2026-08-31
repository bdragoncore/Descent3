/**
 * File system helpers for matching PNG and MD5 files to tests.
 *
 * @module utils/fileHelpers
 */

import {readFile} from 'fs/promises';
import {basename, join} from 'path';

/**
 * Finds a PNG file matching the test name.
 *
 * Uses pattern: Test{Suite}_{Test}_*.png
 *
 * @param testSuite - Test suite name
 * @param testName - Test name
 * @param pngFiles - List of available PNG file paths
 * @returns Matching PNG filename or null
 */
export function findMatchingPng(
  testSuite: string,
  testName: string,
  pngFiles: string[],
): string | null {
  const pattern = `Test${testSuite}_${testName}_`;

  for (const png of pngFiles) {
    const filename = basename(png);
    if (filename.startsWith(pattern)) {
      return filename;
    }
  }

  // Fallback: try old patterns for backwards compatibility
  const oldPatterns = [
    `Test${testName}.png`,
    `Test${testName}Test.png`,
    `Test${testName.replace('Render', '')}.png`,
    `Test${testName.replace('Test', '')}.png`,
  ];

  for (const png of pngFiles) {
    const filename = basename(png);
    if (oldPatterns.includes(filename)) {
      return filename;
    }
  }

  return null;
}

/**
 * Reads MD5 hash from file.
 *
 * @param md5FilePath - Path to MD5 file
 * @returns MD5 hash string or null if error
 */
export async function readMd5File(md5FilePath: string): Promise<string | null> {
  try {
    const content = await readFile(md5FilePath, 'utf-8');
    return content.trim();
  } catch {
    return null;
  }
}

/**
 * Finds baseline MD5 file for a test.
 *
 * @param testName - Test name
 * @param baselineDir - Baseline directory path
 * @returns Path to baseline MD5 file or null
 */
export function findBaselineMd5Path(
  testName: string,
  baselineDir: string | null,
): string | null {
  if (!baselineDir) return null;
  return join(baselineDir, `Test${testName}.md5`);
}

/**
 * Reads and compares MD5 hashes.
 *
 * @param currentPath - Path to current MD5 file
 * @param baselinePath - Path to baseline MD5 file
 * @returns Comparison result
 */
export async function compareMd5(
  currentPath: string,
  baselinePath: string | null,
): Promise<{
  current: string | null;
  baseline: string | null;
  match: boolean | null;
}> {
  const current = await readMd5File(currentPath);

  if (!current) {
    return {current: null, baseline: null, match: null};
  }

  if (!baselinePath) {
    return {current, baseline: null, match: null};
  }

  const baseline = await readMd5File(baselinePath);

  if (!baseline) {
    return {current, baseline: null, match: null};
  }

  return {
    current,
    baseline,
    match: current === baseline,
  };
}
