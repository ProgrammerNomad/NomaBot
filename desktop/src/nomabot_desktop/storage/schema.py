"""SQLite schema definitions and migrations."""

from __future__ import annotations

import sqlite3

SCHEMA_VERSION = 1

MIGRATIONS: dict[int, list[str]] = {
    1: [
        """
        CREATE TABLE IF NOT EXISTS schema_version (
            version INTEGER NOT NULL
        )
        """,
        """
        CREATE TABLE IF NOT EXISTS settings (
            key TEXT PRIMARY KEY,
            value TEXT NOT NULL
        )
        """,
        """
        CREATE TABLE IF NOT EXISTS devices (
            device_id TEXT PRIMARY KEY,
            name TEXT NOT NULL,
            transport_type TEXT NOT NULL,
            transport_config TEXT NOT NULL,
            character_id TEXT NOT NULL DEFAULT 'nomabot',
            last_seen INTEGER,
            firmware_version TEXT,
            protocol_version INTEGER,
            display_width INTEGER,
            display_height INTEGER,
            serial_number TEXT,
            online INTEGER NOT NULL DEFAULT 0
        )
        """,
        """
        CREATE TABLE IF NOT EXISTS scheduler_jobs (
            job_id TEXT PRIMARY KEY,
            action TEXT NOT NULL,
            parameters TEXT NOT NULL,
            interval_seconds INTEGER NOT NULL,
            priority TEXT NOT NULL DEFAULT 'LOW',
            enabled INTEGER NOT NULL DEFAULT 1
        )
        """,
    ],
}


def migrate(conn: sqlite3.Connection) -> None:
    row = conn.execute(
        "SELECT name FROM sqlite_master WHERE type='table' AND name='schema_version'"
    ).fetchone()
    if row is None:
        for stmt in MIGRATIONS[1]:
            conn.execute(stmt)
        conn.execute("INSERT INTO schema_version (version) VALUES (?)", (SCHEMA_VERSION,))
        conn.commit()
        return

    version = conn.execute("SELECT version FROM schema_version").fetchone()[0]
    for v in range(version + 1, SCHEMA_VERSION + 1):
        for stmt in MIGRATIONS.get(v, []):
            conn.execute(stmt)
        conn.execute("UPDATE schema_version SET version = ?", (v,))
    conn.commit()
