/**
 * Main report page component.
 *
 * @module components/ReportPage
 */

import React from 'react';
import {TestResultCard} from './TestResultCard';
import type {TestReportData} from '../types';

/**
 * Props for ReportPage component.
 */
interface ReportPageProps {
  /** ISO timestamp when report was generated */
  generatedAt: string;
  /** Whether tracing was enabled during test run */
  tracingEnabled: boolean;
  /** Total number of tests */
  total: number;
  /** Number of passed tests */
  passed: number;
  /** Number of failed tests */
  failed: number;
  /** Number of MD5 regressions */
  md5Regressions: number;
  /** All test results */
  results: TestReportData[];
  /** Whether baseline was updated */
  updateBaseline?: boolean;
}

/**
 * Main report page displaying test results summary and individual test cards.
 *
 * @param props - Component props
 * @returns React element
 */
export const ReportPage: React.FC<ReportPageProps> = ({
  generatedAt,
  tracingEnabled,
  total,
  passed,
  failed,
  md5Regressions,
  results,
  updateBaseline,
}) => {
  const formatDate = (isoString: string): string => {
    const date = new Date(isoString);
    return date.toLocaleString();
  };

  return (
    <div className="container">
      <h1>Render Test Report</h1>

      {/* Summary Section */}
      <div className="card">
        {/* Header Info */}
        <div
          style={{
            display: 'flex',
            gap: '24px',
            marginBottom: '16px',
            fontSize: '0.875rem',
            color: 'var(--color-text-muted)',
          }}
        >
          <span>Generated: {formatDate(generatedAt)}</span>
          <span>Tracing: {tracingEnabled ? 'Enabled' : 'Disabled'}</span>
        </div>

        {/* Baseline Update Message */}
        {updateBaseline && (
          <div
            style={{
              marginBottom: '16px',
              padding: '12px 16px',
              backgroundColor: 'var(--color-success-bg)',
              color: 'var(--color-success)',
              border: '1px solid var(--color-success)',
              borderRadius: '8px',
              fontWeight: 500,
            }}
          >
            Baseline updated with current MD5 hashes
          </div>
        )}

        {/* Stats Grid */}
        <div
          style={{
            display: 'grid',
            gridTemplateColumns: 'repeat(4, 1fr)',
            gap: '24px',
          }}
        >
          {/* Total */}
          <div
            style={{
              padding: '20px',
              backgroundColor: 'var(--color-bg-tertiary)',
              borderRadius: '12px',
              border: '1px solid var(--color-border)',
              textAlign: 'center',
            }}
          >
            <div
              style={{
                fontSize: '2.5rem',
                fontWeight: 700,
                color: 'var(--color-text-primary)',
                marginBottom: '8px',
              }}
            >
              {total}
            </div>
            <div
              style={{
                fontSize: '0.75rem',
                textTransform: 'uppercase',
                letterSpacing: '1px',
                color: 'var(--color-text-muted)',
              }}
            >
              Total Tests
            </div>
          </div>

          {/* Passed */}
          <div
            style={{
              padding: '20px',
              backgroundColor: 'var(--color-bg-tertiary)',
              borderRadius: '12px',
              border: '1px solid var(--color-border)',
              textAlign: 'center',
            }}
          >
            <div
              style={{
                fontSize: '2.5rem',
                fontWeight: 700,
                color: 'var(--color-success)',
                marginBottom: '8px',
              }}
            >
              {passed}
            </div>
            <div
              style={{
                fontSize: '0.75rem',
                textTransform: 'uppercase',
                letterSpacing: '1px',
                color: 'var(--color-text-muted)',
              }}
            >
              Passed
            </div>
          </div>

          {/* Failed */}
          <div
            style={{
              padding: '20px',
              backgroundColor: 'var(--color-bg-tertiary)',
              borderRadius: '12px',
              border: '1px solid var(--color-border)',
              textAlign: 'center',
            }}
          >
            <div
              style={{
                fontSize: '2.5rem',
                fontWeight: 700,
                color: 'var(--color-error)',
                marginBottom: '8px',
              }}
            >
              {failed}
            </div>
            <div
              style={{
                fontSize: '0.75rem',
                textTransform: 'uppercase',
                letterSpacing: '1px',
                color: 'var(--color-text-muted)',
              }}
            >
              Failed
            </div>
          </div>

          {/* Regressions */}
          <div
            style={{
              padding: '20px',
              backgroundColor: 'var(--color-bg-tertiary)',
              borderRadius: '12px',
              border: '1px solid var(--color-border)',
              textAlign: 'center',
            }}
          >
            <div
              style={{
                fontSize: '2.5rem',
                fontWeight: 700,
                color: 'var(--color-warning)',
                marginBottom: '8px',
              }}
            >
              {md5Regressions}
            </div>
            <div
              style={{
                fontSize: '0.75rem',
                textTransform: 'uppercase',
                letterSpacing: '1px',
                color: 'var(--color-text-muted)',
              }}
            >
              MD5 Regressions
            </div>
          </div>
        </div>
      </div>

      {/* Test Results */}
      {results.map((result, index) => (
        <TestResultCard key={index} result={result} />
      ))}
    </div>
  );
};
