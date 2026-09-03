# ASDEVLAB offline simulation

This directory holds offline fixtures and test inputs for the coordination layer.

Use it to exercise autofocus, plate-solving, and alignment loops without real hardware.

Structure:

- `frames/` — sample camera frames used in tests.
- `mount_state.json` — mock mount coordinates over time (used to simulate goto→capture→solve loops).
- `target.json` — target metadata (ra/dec/width/height) for tests.

Files here are minimal fixtures to exercise services locally. Do not treat these as production data.
