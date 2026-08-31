/**
 * Combined test report page with Unit and Render tabs.
 *
 * @module components/CombinedReportPage
 */

import React from 'react';
import type {TestReportData, UnitTestReportData} from '../types';

/**
 * Props for CombinedReportPage component.
 */
interface CombinedReportPageProps {
  /** ISO timestamp when report was generated */
  generatedAt: string;
  /** Total unit tests */
  unitTotal: number;
  /** Passed unit tests */
  unitPassed: number;
  /** Failed unit tests */
  unitFailed: number;
  /** Unit test results */
  unitResults: UnitTestReportData[];
  /** Total render tests */
  renderTotal: number;
  /** Passed render tests */
  renderPassed: number;
  /** Failed render tests */
  renderFailed: number;
  /** Render test results */
  renderResults: TestReportData[];
  /** Whether tracing was enabled for render tests */
  tracingEnabled: boolean;
  /** Number of MD5 regressions */
  md5Regressions: number;
}

/**
 * Combined test report page with tabbed interface.
 * Uses CSS classes for tab switching (handled by client-side JS).
 *
 * @param props - Component props
 * @returns React element
 */
export const CombinedReportPage: React.FC<CombinedReportPageProps> = ({
  generatedAt,
  unitTotal,
  unitPassed,
  unitFailed,
  unitResults,
  renderTotal,
  renderPassed,
  renderFailed,
  renderResults,
  tracingEnabled,
  md5Regressions,
}) => {
  const formatDate = (isoString: string): string => {
    const date = new Date(isoString);
    return date.toLocaleString();
  };

  const totalTests = unitTotal + renderTotal;
  const totalPassed = unitPassed + renderPassed;
  const totalFailed = unitFailed + renderFailed;

  return (
    <div className="container">
      {/* Header */}
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
        <h1 style={{margin: 0}}>Test Report</h1>
        <div style={{color: 'var(--color-text-muted)', fontSize: '0.875rem'}}>
          Generated: {formatDate(generatedAt)}
        </div>
      </div>

      {/* Overall Summary */}
      <div className="card">
        <div
          style={{
            display: 'grid',
            gridTemplateColumns: 'repeat(4, 1fr)',
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
              {totalTests}
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
              {totalPassed}
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
              {totalFailed}
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
                color:
                  md5Regressions > 0
                    ? 'var(--color-warning)'
                    : 'var(--color-success)',
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

      {/* Tabs */}
      <div
        style={{
          display: 'flex',
          gap: '8px',
          marginBottom: '20px',
          borderBottom: '2px solid var(--color-border)',
        }}
      >
        <button
          id="tab-unit"
          className="tab-button active"
          data-tab="unit"
          style={{
            padding: '12px 24px',
            backgroundColor: 'var(--color-bg-secondary)',
            border: 'none',
            borderBottom: '2px solid var(--color-accent)',
            color: 'var(--color-text-primary)',
            fontSize: '0.9375rem',
            fontWeight: 500,
            cursor: 'pointer',
            marginBottom: '-2px',
            transition: 'all 0.2s ease',
          }}
        >
          Unit Tests ({unitTotal})
        </button>
        <button
          id="tab-render"
          className="tab-button"
          data-tab="render"
          style={{
            padding: '12px 24px',
            backgroundColor: 'transparent',
            border: 'none',
            borderBottom: '2px solid transparent',
            color: 'var(--color-text-muted)',
            fontSize: '0.9375rem',
            fontWeight: 500,
            cursor: 'pointer',
            marginBottom: '-2px',
            transition: 'all 0.2s ease',
          }}
        >
          Render Tests ({renderTotal})
          {tracingEnabled && (
            <span
              style={{
                marginLeft: '8px',
                padding: '2px 8px',
                backgroundColor: 'var(--color-accent)',
                borderRadius: '4px',
                fontSize: '0.75rem',
              }}
            >
              Trace
            </span>
          )}
        </button>
      </div>

      {/* Unit Tests Tab */}
      <div id="content-unit" className="tab-content active">
        {/* Unit Summary */}
        <div className="card" style={{marginBottom: '20px'}}>
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
                  fontSize: '2rem',
                  fontWeight: 700,
                  color: 'var(--color-text-primary)',
                  marginBottom: '8px',
                }}
              >
                {unitTotal}
              </div>
              <div
                style={{
                  fontSize: '0.75rem',
                  textTransform: 'uppercase',
                  letterSpacing: '1px',
                  color: 'var(--color-text-muted)',
                }}
              >
                Total
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
                  fontSize: '2rem',
                  fontWeight: 700,
                  color: 'var(--color-success)',
                  marginBottom: '8px',
                }}
              >
                {unitPassed}
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
                  fontSize: '2rem',
                  fontWeight: 700,
                  color: 'var(--color-error)',
                  marginBottom: '8px',
                }}
              >
                {unitFailed}
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

        {/* Unit Results Table */}
        <div className="card">
          <h2
            style={{
              fontSize: '1.25rem',
              marginBottom: '16px',
              color: 'var(--color-text-primary)',
            }}
          >
            Unit Test Results
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
              {unitResults.map((row, index) => (
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

      {/* Render Tests Tab */}
      <div id="content-render" className="tab-content" style={{display: 'none'}}>
        {/* Render Summary */}
        <div className="card" style={{marginBottom: '20px'}}>
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
                  fontSize: '2rem',
                  fontWeight: 700,
                  color: 'var(--color-text-primary)',
                  marginBottom: '8px',
                }}
              >
                {renderTotal}
              </div>
              <div
                style={{
                  fontSize: '0.75rem',
                  textTransform: 'uppercase',
                  letterSpacing: '1px',
                  color: 'var(--color-text-muted)',
                }}
              >
                Total
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
                  fontSize: '2rem',
                  fontWeight: 700,
                  color: 'var(--color-success)',
                  marginBottom: '8px',
                }}
              >
                {renderPassed}
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
                  fontSize: '2rem',
                  fontWeight: 700,
                  color: 'var(--color-error)',
                  marginBottom: '8px',
                }}
              >
                {renderFailed}
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

        {/* Render Results Cards */}
        <div style={{display: 'flex', flexDirection: 'column', gap: '20px'}}>
          {renderResults.map((test, index) => (
            <div
              key={index}
              className="card"
              style={{
                borderLeft: `4px solid ${
                  test.passed ? 'var(--color-success)' : 'var(--color-error)'
                }`,
              }}
            >
              <div
                style={{
                  display: 'flex',
                  justifyContent: 'space-between',
                  alignItems: 'flex-start',
                  marginBottom: '16px',
                }}
              >
                <div>
                  <div
                    style={{
                      fontFamily: 'var(--font-mono)',
                      fontSize: '0.875rem',
                      color: 'var(--color-text-muted)',
                      marginBottom: '4px',
                    }}
                  >
                    {test.executable}
                  </div>
                  <h3
                    style={{
                      fontSize: '1.125rem',
                      margin: 0,
                      color: 'var(--color-text-primary)',
                    }}
                  >
                    {test.testSuite}.{test.testName}
                  </h3>
                </div>
                <div style={{display: 'flex', gap: '8px', alignItems: 'center'}}>
                  <span
                    style={{
                      padding: '4px 12px',
                      borderRadius: '6px',
                      fontWeight: 600,
                      fontSize: '0.75rem',
                      textTransform: 'uppercase',
                      ...(test.passed
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
                    {test.status}
                  </span>
                  <span
                    style={{
                      fontSize: '0.875rem',
                      color: 'var(--color-text-muted)',
                      fontFamily: 'var(--font-mono)',
                    }}
                  >
                    {test.durationMs}ms
                  </span>
                </div>
              </div>

              {/* MD5 Info */}
              {test.md5Hash && (
                <div
                  style={{
                    marginBottom: '12px',
                    padding: '8px 12px',
                    backgroundColor: 'var(--color-bg-tertiary)',
                    borderRadius: '6px',
                    fontFamily: 'var(--font-mono)',
                    fontSize: '0.75rem',
                    color: 'var(--color-text-muted)',
                  }}
                >
                  MD5: {test.md5Hash}
                </div>
              )}

              {/* PNG Image */}
              {test.pngFilename && (
                <div
                  style={{
                    marginTop: '16px',
                    textAlign: 'center',
                    padding: '16px',
                    backgroundColor: 'var(--color-bg-tertiary)',
                    borderRadius: '12px',
                    border: '1px solid var(--color-border)',
                  }}
                >
                  <img
                    src={`render_output/${test.pngFilename}`}
                    alt={test.testName}
                    style={{
                      maxWidth: '100%',
                      maxHeight: '400px',
                      border: '1px solid var(--color-border)',
                      borderRadius: '8px',
                    }}
                  />
                </div>
              )}

              {/* Trace Button */}
              {test.traceData && (
                <div style={{marginTop: '12px'}}>
                  <button
                    className="trace-btn"
                    data-test-name={`${test.testSuite}.${test.testName}`}
                    data-trace={JSON.stringify(test.traceData)}
                    style={{
                      backgroundColor: 'var(--color-bg-tertiary)',
                      color: 'var(--color-accent-blue)',
                      border: '1px solid var(--color-accent-blue)',
                      padding: '6px 14px',
                      borderRadius: '6px',
                      fontSize: '0.875rem',
                      fontWeight: 500,
                      cursor: 'pointer',
                    }}
                  >
                    Trace
                  </button>
                </div>
              )}

              {/* Callgraph Links */}
              {test.callgraphs && test.callgraphs.length > 0 && (
                <div style={{marginTop: '12px', display: 'flex', gap: '12px'}}>
                  {test.callgraphs.map((cg, cgIndex) => (
                    <a
                      key={cgIndex}
                      href={`render_output/${cg}`}
                      style={{
                        color: 'var(--color-accent-blue)',
                        fontSize: '0.875rem',
                        textDecoration: 'none',
                      }}
                    >
                      → Call Graph {cgIndex + 1}
                    </a>
                  ))}
                </div>
              )}
            </div>
          ))}
        </div>
      </div>

      {/* Trace Modal */}
      <div id="trace-modal" className="modal" style={{display: 'none'}}>
        <div className="modal-backdrop" style={{
          position: 'fixed',
          top: 0,
          left: 0,
          right: 0,
          bottom: 0,
          backgroundColor: 'rgba(0, 0, 0, 0.8)',
          zIndex: 1000,
        }} />
        <div className="modal-content" style={{
          position: 'fixed',
          top: '50%',
          left: '50%',
          transform: 'translate(-50%, -50%)',
          width: '90%',
          maxWidth: '1200px',
          maxHeight: '90vh',
          backgroundColor: 'var(--color-bg-secondary)',
          border: '1px solid var(--color-border)',
          borderRadius: '12px',
          zIndex: 1001,
          display: 'flex',
          flexDirection: 'column',
        }}>
          <div className="modal-header" style={{
            display: 'flex',
            justifyContent: 'space-between',
            alignItems: 'center',
            padding: '16px 24px',
            borderBottom: '1px solid var(--color-border)',
          }}>
            <h2 id="modal-title" style={{margin: 0, fontSize: '1.25rem'}}>Trace</h2>
            <button
              id="modal-close"
              style={{
                background: 'none',
                border: 'none',
                color: 'var(--color-text-muted)',
                fontSize: '1.5rem',
                cursor: 'pointer',
                padding: '0 4px',
              }}
            >
              ×
            </button>
          </div>
          <div className="modal-body" style={{
            flex: 1,
            overflow: 'auto',
            padding: '20px',
          }}>
            <div id="tree-controls" style={{marginBottom: '12px'}}>
              <button id="reset-zoom-btn" style={{
                backgroundColor: 'var(--color-bg-tertiary)',
                color: 'var(--color-text-primary)',
                border: '1px solid var(--color-border)',
                padding: '6px 14px',
                borderRadius: '6px',
                cursor: 'pointer',
                fontSize: '0.875rem',
              }}>Reset Zoom</button>
            </div>
            <div id="modal-tree-container" style={{
              backgroundColor: 'var(--color-bg-tertiary)',
              border: '1px solid var(--color-border)',
              borderRadius: '8px',
              minHeight: '500px',
            }}>
              <svg id="modal-tree-svg" />
            </div>
          </div>
        </div>
      </div>
    </div>
  );
};
