import sqlite3
import json
from dataclasses import dataclass, asdict
from datetime import datetime
from pathlib import Path
from typing import Optional


@dataclass
class TestRun:
    id: Optional[int]
    timestamp: str
    test_name: str
    test_suite: str
    executable: str
    duration_ms: int
    md5_hash: Optional[str]
    status: str
    output_text: str
    environment: dict

    def to_dict(self):
        d = asdict(self)
        d["environment"] = (
            json.dumps(d["environment"])
            if isinstance(d["environment"], dict)
            else d["environment"]
        )
        return d


@dataclass
class RenderFunctionCall:
    test_run_id: int
    function_name: str
    call_count: int
    first_call_time_us: Optional[int]
    last_call_time_us: Optional[int]
    parameters: dict

    def to_dict(self):
        d = asdict(self)
        d["parameters"] = (
            json.dumps(d["parameters"])
            if isinstance(d["parameters"], dict)
            else d["parameters"]
        )
        return d


class RenderTestDatabase:
    def __init__(self, db_path: str = "render_db/render_tests.db"):
        self.db_path = Path(db_path)
        self._init_db()

    def _init_db(self):
        self.db_path.parent.mkdir(parents=True, exist_ok=True)
        with self._get_conn() as conn:
            conn.executescript("""
                CREATE TABLE IF NOT EXISTS test_runs (
                    id INTEGER PRIMARY KEY AUTOINCREMENT,
                    timestamp TEXT NOT NULL DEFAULT (datetime('now')),
                    test_name TEXT NOT NULL,
                    test_suite TEXT NOT NULL,
                    executable TEXT NOT NULL,
                    duration_ms INTEGER NOT NULL,
                    md5_hash TEXT,
                    status TEXT NOT NULL CHECK (status IN ('PASSED', 'FAILED')),
                    output_text TEXT,
                    environment TEXT
                );

                CREATE TABLE IF NOT EXISTS render_functions (
                    id INTEGER PRIMARY KEY AUTOINCREMENT,
                    test_run_id INTEGER NOT NULL REFERENCES test_runs(id) ON DELETE CASCADE,
                    function_name TEXT NOT NULL,
                    call_count INTEGER NOT NULL DEFAULT 1,
                    first_call_time_us INTEGER,
                    last_call_time_us INTEGER,
                    parameters TEXT
                );

                CREATE INDEX IF NOT EXISTS idx_render_functions_test_run ON render_functions(test_run_id);
                CREATE INDEX IF NOT EXISTS idx_test_runs_timestamp ON test_runs(timestamp);
                CREATE INDEX IF NOT EXISTS idx_test_runs_name ON test_runs(test_name);
            """)

    def _get_conn(self):
        return sqlite3.connect(str(self.db_path))

    def insert_test_run(self, test_run: TestRun) -> int:
        with self._get_conn() as conn:
            cursor = conn.cursor()
            env_json = (
                json.dumps(test_run.environment)
                if isinstance(test_run.environment, dict)
                else test_run.environment
            )
            cursor.execute(
                """
                INSERT INTO test_runs (timestamp, test_name, test_suite, executable, duration_ms, md5_hash, status, output_text, environment)
                VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)
            """,
                (
                    test_run.timestamp,
                    test_run.test_name,
                    test_run.test_suite,
                    test_run.executable,
                    test_run.duration_ms,
                    test_run.md5_hash,
                    test_run.status,
                    test_run.output_text,
                    env_json,
                ),
            )
            conn.commit()
            return cursor.lastrowid

    def insert_render_functions(self, calls: list[RenderFunctionCall]):
        if not calls:
            return
        with self._get_conn() as conn:
            cursor = conn.cursor()
            for call in calls:
                params_json = (
                    json.dumps(call.parameters)
                    if isinstance(call.parameters, dict)
                    else call.parameters
                )
                cursor.execute(
                    """
                    INSERT INTO render_functions (test_run_id, function_name, call_count, first_call_time_us, last_call_time_us, parameters)
                    VALUES (?, ?, ?, ?, ?, ?)
                """,
                    (
                        call.test_run_id,
                        call.function_name,
                        call.call_count,
                        call.first_call_time_us,
                        call.last_call_time_us,
                        params_json,
                    ),
                )
            conn.commit()

    def get_previous_run(self, test_name: str) -> Optional[TestRun]:
        with self._get_conn() as conn:
            conn.row_factory = sqlite3.Row
            cursor = conn.cursor()
            cursor.execute(
                """
                SELECT id, timestamp, test_name, test_suite, executable, duration_ms, md5_hash, status, output_text, environment
                FROM test_runs
                WHERE test_name = ?
                ORDER BY timestamp DESC
                LIMIT 1
            """,
                (test_name,),
            )
            row = cursor.fetchone()
            if row:
                return TestRun(
                    id=row["id"],
                    timestamp=row["timestamp"],
                    test_name=row["test_name"],
                    test_suite=row["test_suite"],
                    executable=row["executable"],
                    duration_ms=row["duration_ms"],
                    md5_hash=row["md5_hash"],
                    status=row["status"],
                    output_text=row["output_text"] or "",
                    environment=json.loads(row["environment"])
                    if row["environment"]
                    else {},
                )
            return None

    def get_all_runs(self, limit: int = 100) -> list[TestRun]:
        with self._get_conn() as conn:
            conn.row_factory = sqlite3.Row
            cursor = conn.cursor()
            cursor.execute(
                """
                SELECT id, timestamp, test_name, test_suite, executable, duration_ms, md5_hash, status, output_text, environment
                FROM test_runs
                ORDER BY timestamp DESC
                LIMIT ?
            """,
                (limit,),
            )
            rows = cursor.fetchall()
            return [
                TestRun(
                    id=row["id"],
                    timestamp=row["timestamp"],
                    test_name=row["test_name"],
                    test_suite=row["test_suite"],
                    executable=row["executable"],
                    duration_ms=row["duration_ms"],
                    md5_hash=row["md5_hash"],
                    status=row["status"],
                    output_text=row["output_text"] or "",
                    environment=json.loads(row["environment"])
                    if row["environment"]
                    else {},
                )
                for row in rows
            ]

    def get_render_functions(self, test_run_id: int) -> list[RenderFunctionCall]:
        with self._get_conn() as conn:
            conn.row_factory = sqlite3.Row
            cursor = conn.cursor()
            cursor.execute(
                """
                SELECT test_run_id, function_name, call_count, first_call_time_us, last_call_time_us, parameters
                FROM render_functions
                WHERE test_run_id = ?
            """,
                (test_run_id,),
            )
            rows = cursor.fetchall()
            return [
                RenderFunctionCall(
                    test_run_id=row["test_run_id"],
                    function_name=row["function_name"],
                    call_count=row["call_count"],
                    first_call_time_us=row["first_call_time_us"],
                    last_call_time_us=row["last_call_time_us"],
                    parameters=json.loads(row["parameters"])
                    if row["parameters"]
                    else {},
                )
                for row in rows
            ]
