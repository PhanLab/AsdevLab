ASDEVLAB Web UI Service
========================

This folder contains the lightweight web UI used to control ASDEVLAB services.

Run locally:

  python web_app.py

Environment variables:
- `ASDEVLAB_ONSTEP_URL` - default OnStep URL (or set via the UI)
- `ASDEVLAB_CAMERA_STREAM_URL` - default camera stream URL
- `ASDEVLAB_WEB_PORT` - default port (8080)

Notes:
- The web UI calls the `build/asdevlab_core_app` binary via subprocess; ensure the project is built and `build/` exists.
- Add runtime dependencies to `requirements.txt` if you introduce non-stdlib packages.
