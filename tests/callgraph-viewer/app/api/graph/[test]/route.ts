import { NextRequest, NextResponse } from "next/server";
import path from "path";
import fs from "fs";

const OUTPUT_DIR =
  process.env.OUTPUT_DIR ?? path.join(process.cwd(), "..", "render", "output");

export async function GET(
  _req: NextRequest,
  { params }: { params: Promise<{ test: string }> }
) {
  const { test } = await params;
  if (!test) {
    return NextResponse.json({ error: "Missing test name" }, { status: 400 });
  }
  const filepath = path.join(OUTPUT_DIR, `${test}__graph.json`);
  if (!fs.existsSync(filepath)) {
    return NextResponse.json({ error: "Not found" }, { status: 404 });
  }
  try {
    const data = JSON.parse(fs.readFileSync(filepath, "utf-8"));
    return NextResponse.json(data);
  } catch (e) {
    return NextResponse.json(
      { error: e instanceof Error ? e.message : "Invalid JSON" },
      { status: 500 }
    );
  }
}
