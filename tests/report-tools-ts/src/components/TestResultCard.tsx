/**
 * Individual test result card component.
 *
 * @module components/TestResultCard
 */

import React from 'react';
import type {TestReportData} from '../types';

/**
 * Props for TestResultCard component.
 */
interface TestResultCardProps {
  /** Test result data */
  result: TestReportData;
}

/**
 * Renders MD5 comparison UI based on current and previous hashes.
 *
 * @param result - Test result data
 * @returns React element for MD5 display
 */
const renderMd5Comparison = (result: TestReportData): React.ReactElement => {
  if (!result.md5Hash) {
    return <span style={{color: 'var(--color-text-muted)'}}>N/A</span>;
  }

  // Has baseline
  if (result.previousMd5) {
    if (result.previousMd5 === result.md5Hash) {
      // Match
      return (
        <span style={{color: 'var(--color-success)', fontWeight: 500}}>
          ✅ {result.md5Hash} (match)
        </span>
      );
    } else {
      // Mismatch
      return (
        <span
          style={{
            backgroundColor: 'var(--color-warning-bg)',
            color: 'var(--color-warning)',
            padding: '6px 12px',
            borderRadius: '6px',
            border: '1px solid var(--color-warning)',
            display: 'inline-block',
          }}
        >
          ⚠️ {result.md5Hash}
          <br />
          Previous: {result.previousMd5}
        </span>
      );
    }
  }

  // No baseline
  return (
    <span style={{color: 'var(--color-warning)', fontWeight: 500}}>
      ➖ {result.md5Hash} (no baseline)
    </span>
  );
};

/**
 * Card displaying detailed information for a single test result.
 *
 * @param props - Component props
 * @returns React element
 */
export const TestResultCard: React.FC<TestResultCardProps> = ({result}) => {
  const isPassed = result.status === 'PASSED';

  // Format environment variables
  const envHtml = Object.entries(result.environment || {})
    .map(([k, v]) => `${k}=${v}`)
    .join(', ') || '(none)';

  return (
    <div className="card">
      {/* Header */}
      <div
        style={{
          display: 'flex',
          justifyContent: 'space-between',
          alignItems: 'center',
          marginBottom: '16px',
          paddingBottom: '16px',
          borderBottom: '1px solid var(--color-border)',
        }}
      >
        <span
          style={{
            fontSize: '1.125rem',
            fontWeight: 600,
            color: 'var(--color-text-primary)',
          }}
        >
          {result.testSuite}.{result.testName}
        </span>
        <span
          style={{
            padding: '8px 16px',
            borderRadius: '8px',
            fontWeight: 600,
            fontSize: '0.875rem',
            textTransform: 'uppercase',
            letterSpacing: '1px',
            ...(isPassed
              ? {
                  backgroundColor: 'var(--color-success-bg)',
                  color: 'var(--color-success)',
                  border: '1px solid var(--color-success)',
                }
              : {
                  backgroundColor: 'var(--color-error-bg)',
                  color: 'var(--color-error)',
                  border: '1px solid var(--color-error)',
                }),
          }}
        >
          {result.status}
        </span>
      </div>

      {/* Details Table */}
      <table className="detail-table">
        <tbody>
          <tr>
            <th>Executable</th>
            <td>{result.executable}</td>
          </tr>
          <tr>
            <th>Test Suite</th>
            <td>{result.testSuite}</td>
          </tr>
          <tr>
            <th>Duration</th>
            <td>{result.durationMs}ms</td>
          </tr>
          <tr>
            <th>MD5 Hash</th>
            <td>{renderMd5Comparison(result)}</td>
          </tr>
          <tr>
            <th>Environment</th>
            <td
              style={{
                fontFamily: 'var(--font-mono)',
                fontSize: '0.875rem',
              }}
            >
              {envHtml}
            </td>
          </tr>
        </tbody>
      </table>

      {/* Trace Link */}
      {result.traceHtmlFilename && (
        <a
          href={result.traceHtmlFilename}
          target="_blank"
          rel="noopener noreferrer"
          className="btn btn-primary"
          style={{marginTop: '16px'}}
        >
          📊 View Call Tree
        </a>
      )}

      {/* Image or "No render output" message */}
      {result.pngFilename ? (
        <div className="test-image">
          <img src={result.pngFilename} alt={result.testName} />
        </div>
      ) : (
        <div
          className="test-image"
          style={{
            color: 'var(--color-text-muted)',
            fontStyle: 'italic',
          }}
        >
          No render output (test did not produce image)
        </div>
      )}
    </div>
  );
};
