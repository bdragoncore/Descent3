import { NextResponse } from "next/server";
import path from "path";
import fs from "fs";

const OUTPUT_DIR =
  process.env.OUTPUT_DIR ?? path.join(process.cwd(), "..", "render", "output");

export async function GET() {
  try {
    if (!fs.existsSync(OUTPUT_DIR)) {
      return NextResponse.json([]);
    }
    const files = fs
      .readdirSync(OUTPUT_DIR)
      .filter((f) => f.endsWith("__graph.json"))
      .map((f) => ({
        filename: f,
        test: f.replace("__graph.json", "").replace(/__/g, "."),
      }));
    return NextResponse.json(files);
  } catch (e) {
    return NextResponse.json(
      { error: e instanceof Error ? e.message : "Failed to list graphs" },
      { status: 500 }
    );
  }
}
