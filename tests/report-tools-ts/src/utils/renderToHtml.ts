/**
 * Utilities for rendering React components to static HTML.
 *
 * @module utils/renderToHtml
 */

import React from 'react';
import {renderToString} from 'react-dom/server';
import {ReportPage} from '../components/ReportPage';
import type {TestReportData} from '../types';

/**
 * CSS styles to inline in generated HTML.
 * This includes the Tailwind-generated styles and custom theme.
 */
const CRITICAL_CSS = `
  /* Base reset */
  *, *::before, *::after {
    box-sizing: border-box;
  }

  /* Dark theme CSS variables */
  :root {
    --color-bg-primary: #0f1117;
    --color-bg-secondary: #161b22;
    --color-bg-tertiary: #21262d;
    --color-text-primary: #f1f5f9;
    --color-text-secondary: #cbd5e1;
    --color-text-muted: #94a3b8;
    --color-accent: #8b5cf6;
    --color-accent-hover: #a78bfa;
    --color-accent-blue: #58a6ff;
    --color-accent-blue-hover: #79c0ff;
    --color-border: #2d3748;
    --color-success: #56d364;
    --color-success-bg: rgba(35, 134, 54, 0.2);
    --color-error: #ff7b72;
    --color-error-bg: rgba(218, 54, 51, 0.2);
    --color-warning: #d29922;
    --color-warning-bg: rgba(158, 106, 3, 0.2);
    --shadow-report: 0 10px 40px rgba(0, 0, 0, 0.4);
    --font-sans: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, Oxygen, Ubuntu, sans-serif;
    --font-mono: 'Menlo', 'Consolas', 'Monaco', monospace;
  }

  :root {
    color-scheme: dark;
  }

  body {
    font-family: var(--font-sans);
    line-height: 1.6;
    margin: 0;
    padding: 20px;
    background-color: var(--color-bg-primary);
    color: var(--color-text-primary);
  }

  .container {
    max-width: 1200px;
    margin: 0 auto;
  }

  h1 {
    color: var(--color-text-primary);
    border-bottom: 2px solid var(--color-accent);
    padding-bottom: 15px;
    font-size: 2.5em;
    font-weight: 700;
    margin-bottom: 30px;
  }

  a {
    color: var(--color-accent-blue);
    text-decoration: none;
    transition: color 0.2s ease;
  }

  a:hover {
    color: var(--color-accent-blue-hover);
  }

  .card {
    background-color: var(--color-bg-secondary);
    border: 1px solid var(--color-border);
    border-radius: 16px;
    box-shadow: var(--shadow-report);
    padding: 30px;
    margin-bottom: 25px;
  }

  .btn {
    display: inline-block;
    padding: 10px 20px;
    border-radius: 8px;
    font-weight: 500;
    transition: all 0.2s ease;
    cursor: pointer;
    border: none;
    font-size: 1rem;
    text-decoration: none;
  }

  .btn-primary {
    background-color: var(--color-accent);
    color: white;
    box-shadow: 0 4px 12px rgba(139, 92, 246, 0.3);
  }

  .btn-primary:hover {
    background-color: var(--color-accent-hover);
    transform: translateY(-2px);
    box-shadow: 0 6px 20px rgba(139, 92, 246, 0.4);
  }

  .detail-table {
    width: 100%;
    border-collapse: collapse;
    margin-bottom: 20px;
  }

  .detail-table th {
    color: var(--color-text-muted);
    font-weight: 500;
    width: 150px;
    text-transform: uppercase;
    font-size: 0.75rem;
    letter-spacing: 1px;
    padding: 12px 15px;
    text-align: left;
    border-bottom: 1px solid var(--color-border);
  }

  .detail-table td {
    color: var(--color-text-secondary);
    padding: 12px 15px;
    border-bottom: 1px solid var(--color-border);
  }

  .test-image {
    margin-top: 20px;
    text-align: center;
    padding: 20px;
    background-color: var(--color-bg-tertiary);
    border-radius: 12px;
    border: 1px solid var(--color-border);
  }

  .test-image img {
    max-width: 100%;
    border: 1px solid var(--color-border);
    border-radius: 8px;
    box-shadow: 0 4px 12px rgba(0, 0, 0, 0.4);
  }
`;

/**
 * Generates the main report HTML from test results.
 *
 * @param data - Report data including stats and test results
 * @returns Complete HTML string
 */
export function generateReportHtml(data: {
  generatedAt: string;
  tracingEnabled: boolean;
  total: number;
  passed: number;
  failed: number;
  md5Regressions: number;
  results: TestReportData[];
  updateBaseline?: boolean;
}): string {
  const htmlContent = renderToString(React.createElement(ReportPage, data));

  return `<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Render Test Report</title>
  <style>${CRITICAL_CSS}</style>
</head>
<body>
  ${htmlContent}
</body>
</html>`;
}

/**
 * Generates a simple HTML page for per-trace visualization.
 *
 * For now, this creates a placeholder that will be replaced with
 * the full D3.js tree view component.
 *
 * @param treeData - Tree data as JSON string
 * @param testName - Name of the test
 * @returns Complete HTML string
 */
export function generateTraceHtml(treeData: string, testName: string): string {
  return `<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>Trace: ${testName}</title>
  <script src="https://cdnjs.cloudflare.com/ajax/libs/d3/7.8.5/d3.min.js"></script>
  <style>${CRITICAL_CSS}</style>
  <style>
    .tree-container {
      background-color: var(--color-bg-secondary);
      border: 1px solid var(--color-border);
      border-radius: 12px;
      min-height: 400px;
      padding: 20px;
    }
    .node rect {
      fill: var(--color-bg-tertiary);
      stroke: var(--color-accent-blue);
      stroke-width: 2;
    }
    .node text {
      fill: var(--color-text-primary);
      font-family: var(--font-mono);
      font-size: 11px;
    }
    .link {
      fill: none;
      stroke: var(--color-border);
      stroke-width: 1.5;
    }
  </style>
</head>
<body>
  <div class="container">
    <h1>Call Tree: ${testName}</h1>
    <div class="card">
      <div id="tree-container" class="tree-container"></div>
    </div>
  </div>
  <script>
    const RAW_TREE = ${treeData};
    // D3.js tree visualization will be initialized here
    console.log('Tree data loaded:', RAW_TREE);
  </script>
</body>
</html>`;
}
