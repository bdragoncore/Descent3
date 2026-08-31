/**
 * Unit test report page (non-render): summary + table of tests.
 *
 * @module components/UnitReportPage
 */

import React from 'react';
import type {UnitTestReportData} from '../types';

/**
 * Props for UnitReportPage component.
 */
interface UnitReportPageProps {
  /** ISO timestamp when report was generated */
  generatedAt: string;
  /** Total number of tests */
  total: number;
  /** Number of passed tests */
  passed: number;
  /** Number of failed tests */
  failed: number;
  /** All unit test results */
  results: UnitTestReportData[];
}

/**
 * Unit test report page: summary stats and a table of results.
 *
 * @param props - Component props
 * @returns React element
 */
export const UnitReportPage: React.FC<UnitReportPageProps> = ({
  generatedAt,
  total,
  passed,
  failed,
  results,
}) => {
  const formatDate = (isoString: string): string => {
    const date = new Date(isoString);
    return date.toLocaleString();
  };

  return (
    <div className="container">
      <div
        style={{
          display: 'flex',
          justifyContent: 'space-between',
          alignItems: 'center',
          flexWrap: 'wrap',
          gap: '12px',
          marginBottom: '8px',
        }}
      >
        <h1 style={{ margin: 0 }}>Unit Test Report</h1>
        <a
          href="render_output/render_report.html"
          style={{
            color: 'var(--color-accent-blue)',
            fontSize: '0.9375rem',
            fontWeight: 500,
          }}
        >
          → Render tests
        </a>
      </div>

      {/* Summary Section */}
      <div className="card">
        <div
          style={{
            marginBottom: '16px',
            fontSize: '0.875rem',
            color: 'var(--color-text-muted)',
          }}
        >
          Generated: {formatDate(generatedAt)}
        </div>

        <div
          style={{
            display: 'grid',
            gridTemplateColumns: 'repeat(3, 1fr)',
            gap: '24px',
          }}
        >
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
        </div>
      </div>

      {/* Results Table */}
      <div className="card">
        <h2
          style={{
            fontSize: '1.25rem',
            marginBottom: '16px',
            color: 'var(--color-text-primary)',
          }}
        >
          Test Results
        </h2>
        <table
          className="detail-table"
          style={{
            width: '100%',
            borderCollapse: 'collapse',
          }}
        >
          <thead>
            <tr>
              <th
                style={{
                  color: 'var(--color-text-muted)',
                  fontWeight: 500,
                  textTransform: 'uppercase',
                  fontSize: '0.75rem',
                  letterSpacing: '1px',
                  padding: '12px 15px',
                  textAlign: 'left',
                  borderBottom: '1px solid var(--color-border)',
                }}
              >
                Executable
              </th>
              <th
                style={{
                  color: 'var(--color-text-muted)',
                  fontWeight: 500,
                  textTransform: 'uppercase',
                  fontSize: '0.75rem',
                  letterSpacing: '1px',
                  padding: '12px 15px',
                  textAlign: 'left',
                  borderBottom: '1px solid var(--color-border)',
                }}
              >
                Suite
              </th>
              <th
                style={{
                  color: 'var(--color-text-muted)',
                  fontWeight: 500,
                  textTransform: 'uppercase',
                  fontSize: '0.75rem',
                  letterSpacing: '1px',
                  padding: '12px 15px',
                  textAlign: 'left',
                  borderBottom: '1px solid var(--color-border)',
                }}
              >
                Test
              </th>
              <th
                style={{
                  color: 'var(--color-text-muted)',
                  fontWeight: 500,
                  textTransform: 'uppercase',
                  fontSize: '0.75rem',
                  letterSpacing: '1px',
                  padding: '12px 15px',
                  textAlign: 'left',
                  borderBottom: '1px solid var(--color-border)',
                }}
              >
                Status
              </th>
              <th
                style={{
                  color: 'var(--color-text-muted)',
                  fontWeight: 500,
                  textTransform: 'uppercase',
                  fontSize: '0.75rem',
                  letterSpacing: '1px',
                  padding: '12px 15px',
                  textAlign: 'right',
                  borderBottom: '1px solid var(--color-border)',
                }}
              >
                Duration
              </th>
            </tr>
          </thead>
          <tbody>
            {results.map((row, index) => (
              <tr key={index}>
                <td
                  style={{
                    color: 'var(--color-text-secondary)',
                    padding: '12px 15px',
                    borderBottom: '1px solid var(--color-border)',
                    fontFamily: 'var(--font-mono)',
                    fontSize: '0.875rem',
                  }}
                >
                  {row.executable}
                </td>
                <td
                  style={{
                    color: 'var(--color-text-secondary)',
                    padding: '12px 15px',
                    borderBottom: '1px solid var(--color-border)',
                  }}
                >
                  {row.testSuite}
                </td>
                <td
                  style={{
                    color: 'var(--color-text-secondary)',
                    padding: '12px 15px',
                    borderBottom: '1px solid var(--color-border)',
                  }}
                >
                  {row.testName}
                </td>
                <td
                  style={{
                    padding: '12px 15px',
                    borderBottom: '1px solid var(--color-border)',
                  }}
                >
                  <span
                    style={{
                      padding: '4px 10px',
                      borderRadius: '6px',
                      fontWeight: 600,
                      fontSize: '0.75rem',
                      textTransform: 'uppercase',
                      ...(row.passed
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
                    {row.status}
                  </span>
                </td>
                <td
                  style={{
                    color: 'var(--color-text-secondary)',
                    padding: '12px 15px',
                    borderBottom: '1px solid var(--color-border)',
                    textAlign: 'right',
                    fontFamily: 'var(--font-mono)',
                    fontSize: '0.875rem',
                  }}
                >
                  {row.durationMs} ms
                </td>
              </tr>
            ))}
          </tbody>
        </table>
      </div>
    </div>
  );
};
