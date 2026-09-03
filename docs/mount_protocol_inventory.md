# OnStepX Firmware LX200 Inventory

This inventory documents the LX200 mount protocol commands implemented by OnStepX firmware and maps them against ASDEVLAB support.

- Total LX200 commands discovered: 162
- Wrapper support coverage: 33/162 (20.4%)
- Parser support coverage: 17/162 (10.5%)
- Test coverage: 31/162 (19.1%)

## Inventory details

The full command list with coverage flags is available in `docs/firmware_lx200_inventory.csv`.

## Coverage classifications

- Wrapper: commands that are emitted or supported by `software/asdevlab/src/hardware/mount/onstep_mount_client.cpp`
- Parser: commands with an explicit typed response parser in `software/asdevlab/src/hardware/mount/lx200_parser.cpp`
- Tests: commands exercised by ASDEVLAB unit tests, either via direct command expectations or parse function coverage
