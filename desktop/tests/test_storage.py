"""Storage service tests."""

from pathlib import Path

from nomabot_desktop.storage.service import DeviceRow, StorageService


def test_storage_device_roundtrip(tmp_path: Path) -> None:
    db = tmp_path / "test.db"
    storage = StorageService(db)
    storage.upsert_device(
        DeviceRow(
            device_id="d1",
            name="Test",
            transport_type="serial",
            transport_config={"port": "COM3", "baud": 115200},
            firmware_version="0.1.0",
            protocol_version=1,
            display_width=170,
            display_height=320,
            serial_number="ABC",
            online=True,
        )
    )
    row = storage.get_device("d1")
    assert row is not None
    assert row.firmware_version == "0.1.0"
    assert row.display_width == 170
    storage.close()


def test_scheduler_job_seed(tmp_path: Path) -> None:
    db = tmp_path / "test.db"
    storage = StorageService(db)
    storage.seed_default_jobs()
    jobs = storage.list_scheduler_jobs()
    assert any(j.job_id == "test-reminder" for j in jobs)
    assert jobs[0].action == "ShowMessage"
    storage.close()
