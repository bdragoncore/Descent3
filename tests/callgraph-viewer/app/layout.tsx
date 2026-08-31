import type { Metadata } from "next";

export const metadata: Metadata = {
  title: "Call Graph Viewer",
  description: "IDA-style call graph viewer for render tests",
};

export default function RootLayout({
  children,
}: {
  children: React.ReactNode;
}) {
  return (
    <html lang="en">
      <body
        style={{
          margin: 0,
          padding: 0,
          background: "#0d1117",
          color: "#c9d1d9",
          fontFamily: "-apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif",
        }}
      >
        {children}
      </body>
    </html>
  );
}
