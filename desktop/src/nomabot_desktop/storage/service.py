"""SQLite storage service."""

from __future__ import annotations

import json
import sqlite3
import time
from dataclasses import dataclass
from pathlib import Path
from typing import Any

from nomabot_desktop.storage.schema import migrate


@dataclass
class DeviceRow:
    device_id: str
    name: str
    transport_type: str
    transport_config: dict[str, Any]
    character_id: str = "nomabot"
    last_seen: int | None = None
    firmware_version: str | None = None
    protocol_version: int | None = None
    display_width: int | None = None
    display_height: int | None = None
    serial_number: str | None = None
    online: bool = False


@dataclass
class SchedulerJobRow:
    job_id: str
    action: str
    parameters: dict[str, Any]
    interval_seconds: int
    priority: str = "LOW"
    enabled: bool = True


class StorageService:
    def __init__(self, db_path: Path) -> None:
        db_path.parent.mkdir(parents=True, exist_ok=True)
        self._conn = sqlite3.connect(db_path, check_same_thread=False)
        self._conn.row_factory = sqlite3.Row
        migrate(self._conn)

    def close(self) -> None:
        self._conn.close()

    def get_setting(self, key: str, default: str | None = None) -> str | None:
        row = self._conn.execute("SELECT value FROM settings WHERE key = ?", (key,)).fetchone()
        return row["value"] if row else default

    def set_setting(self, key: str, value: str) -> None:
        self._conn.execute(
            """
            INSERT INTO settings (key, value) VALUES (?, ?)
            ON CONFLICT(key) DO UPDATE SET value = excluded.value
            """,
            (key, value),
        )
        self._conn.commit()

    def upsert_device(self, device: DeviceRow) -> None:
        self._conn.execute(
            """
            INSERT INTO devices (
                device_id, name, transport_type, transport_config, character_id,
                last_seen, firmware_version, protocol_version,
                display_width, display_height, serial_number, online
            ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
            ON CONFLICT(device_id) DO UPDATE SET
                name = excluded.name,
                transport_type = excluded.transport_type,
                transport_config = excluded.transport_config,
                character_id = excluded.character_id,
                last_seen = excluded.last_seen,
                firmware_version = excluded.firmware_version,
                protocol_version = excluded.protocol_version,
                display_width = excluded.display_width,
                display_height = excluded.display_height,
                serial_number = excluded.serial_number,
                online = excluded.online
            """,
            (
                device.device_id,
                device.name,
                device.transport_type,
                json.dumps(device.transport_config),
                device.character_id,
                device.last_seen or int(time.time() * 1000),
                device.firmware_version,
                device.protocol_version,
                device.display_width,
                device.display_height,
                device.serial_number,
                1 if device.online else 0,
            ),
        )
        self._conn.commit()

    def get_device(self, device_id: str) -> DeviceRow | None:
        row = self._conn.execute(
            "SELECT * FROM devices WHERE device_id = ?", (device_id,)
        ).fetchone()
        if not row:
            return None
        return self._row_to_device(row)

    def get_default_device(self) -> DeviceRow | None:
        row = self._conn.execute(
            "SELECT * FROM devices ORDER BY last_seen DESC LIMIT 1"
        ).fetchone()
        if not row:
            return None
        return self._row_to_device(row)

    def list_scheduler_jobs(self) -> list[SchedulerJobRow]:
        rows = self._conn.execute("SELECT * FROM scheduler_jobs").fetchall()
        return [self._row_to_job(r) for r in rows]

    def upsert_scheduler_job(self, job: SchedulerJobRow) -> None:
        self._conn.execute(
            """
            INSERT INTO scheduler_jobs (
                job_id, action, parameters, interval_seconds, priority, enabled
            )
            VALUES (?, ?, ?, ?, ?, ?)
            ON CONFLICT(job_id) DO UPDATE SET
                action = excluded.action,
                parameters = excluded.parameters,
                interval_seconds = excluded.interval_seconds,
                priority = excluded.priority,
                enabled = excluded.enabled
            """,
            (
                job.job_id,
                job.action,
                json.dumps(job.parameters),
                job.interval_seconds,
                job.priority,
                1 if job.enabled else 0,
            ),
        )
        self._conn.commit()

    def seed_default_jobs(self) -> None:
        existing = self.list_scheduler_jobs()
        if existing:
            return
        self.upsert_scheduler_job(
            SchedulerJobRow(
                job_id="test-reminder",
                action="ShowMessage",
                parameters={"text": "Hydration check"},
                interval_seconds=120,
                priority="LOW",
                enabled=True,
            )
        )

    @staticmethod
    def _row_to_device(row: sqlite3.Row) -> DeviceRow:
        return DeviceRow(
            device_id=row["device_id"],
            name=row["name"],
            transport_type=row["transport_type"],
            transport_config=json.loads(row["transport_config"]),
            character_id=row["character_id"],
            last_seen=row["last_seen"],
            firmware_version=row["firmware_version"],
            protocol_version=row["protocol_version"],
            display_width=row["display_width"],
            display_height=row["display_height"],
            serial_number=row["serial_number"],
            online=bool(row["online"]),
        )

    @staticmethod
    def _row_to_job(row: sqlite3.Row) -> SchedulerJobRow:
        return SchedulerJobRow(
            job_id=row["job_id"],
            action=row["action"],
            parameters=json.loads(row["parameters"]),
            interval_seconds=row["interval_seconds"],
            priority=row["priority"],
            enabled=bool(row["enabled"]),
        )
