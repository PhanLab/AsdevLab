import http.server
import os
import shlex
import subprocess
import urllib.parse
from pathlib import Path

ROOT = Path(__file__).resolve().parent
DEFAULT_ONSTEP_URL = os.environ.get("ASDEVLAB_ONSTEP_URL", "http://esp32.local")
DEFAULT_CAMERA_STREAM_URL = os.environ.get("ASDEVLAB_CAMERA_STREAM_URL", "")

HTML = f"""
<!doctype html>
<html>
<head>
  <meta charset="utf-8" />
  <title>ASDEVLAB Control</title>
  <style>
    body {{ font-family: Arial, sans-serif; margin: 1rem; background: #0f172a; color: #e2e8f0; }}
    .grid {{ display: grid; grid-template-columns: 1.2fr 0.8fr; gap: 1rem; }}
    .card {{ background: #111827; border: 1px solid #334155; border-radius: 10px; padding: 1rem; margin-bottom: 1rem; }}
    button {{ margin: 0.2rem; padding: 0.5rem 0.75rem; border: 0; border-radius: 6px; cursor: pointer; background: #2563eb; color: white; }}
    button.secondary {{ background: #475569; }}
    button.danger {{ background: #dc2626; }}
    .status {{ display: flex; gap: 0.5rem; flex-wrap: wrap; margin-bottom: 0.8rem; }}
    .pill {{ background: #1f2937; border: 1px solid #475569; padding: 0.25rem 0.55rem; border-radius: 999px; font-size: 0.9rem; }}
    .live {{ width: 100%; min-height: 280px; background: #000; border: 1px solid #334155; border-radius: 8px; display: flex; align-items: center; justify-content: center; color: #94a3b8; overflow: hidden; }}
    .live img {{ max-width: 100%; max-height: 280px; }}
    pre {{ background: #020617; border: 1px solid #334155; padding: 0.8rem; border-radius: 8px; white-space: pre-wrap; }}
    select, input {{ padding: 0.45rem; border-radius: 6px; margin-right: 0.3rem; }}
  </style>
</head>
<body>
  <h1>ASDEVLAB Telescope Control</h1>
  <div class="status">
    <span class="pill">Mount: Idle</span>
    <span class="pill">Tracking: Off</span>
    <span class="pill">Camera: None</span>
    <span class="pill">LiveStack: Off</span>
  </div>
  <div class="card">
    <h2>Connection</h2>
    <label>OnStepX URL: <input id="onstepUrl" value="{DEFAULT_ONSTEP_URL}" size="40" /></label>
    <label>Camera stream URL: <input id="streamUrl" value="{DEFAULT_CAMERA_STREAM_URL}" size="40" /></label>
    <button onclick="applyConfig()">Apply</button>
  </div>
  <div class="grid">
    <div>
      <div class="card">
        <h2>Mount / Telescope</h2>
        <button onclick="run('motion goto M31')">Goto M31</button>
        <button onclick="run('motion move north 100')">N 100</button>
        <button onclick="run('motion move south 100')">S 100</button>
        <button onclick="run('motion move east 100')">E 100</button>
        <button onclick="run('motion move west 100')">W 100</button>
        <button class="secondary" onclick="run('motion park')">Park</button>
        <button class="danger" onclick="run('motion stop')">Stop</button>
        <button class="secondary" onclick="run('tracking start')">Tracking On</button>
        <button class="secondary" onclick="run('tracking stop')">Tracking Off</button>
      </div>
      <div class="card">
        <h2>Camera / Imaging</h2>
        <label>Camera: <select id="cameraSel"><option value="uvc:0">UVC</option><option value="android:0">Android</option></select></label>
        <button onclick="openCamera()">Open</button>
        <button onclick="run('camera start')">Start Stream</button>
        <button onclick="run('camera stop')">Stop Stream</button>
        <button onclick="run('image start')">Start Live Stack</button>
        <button class="danger" onclick="run('image stop')">Stop Live Stack</button>
      </div>
      <div class="card">
        <h2>Log</h2>
        <pre id="out">Ready.</pre>
      </div>
    </div>
    <div>
      <div class="card">
        <h2>Live View</h2>
        <div class="live">
          <img id="preview" src="{DEFAULT_CAMERA_STREAM_URL}" alt="camera preview" onerror="this.style.display='none'; this.nextElementSibling.style.display='flex';" />
          <div id="previewPlaceholder" style="display: {'flex' if not DEFAULT_CAMERA_STREAM_URL else 'none'}; align-items: center; justify-content: center; width: 100%; height: 100%;">Camera preview placeholder</div>
        </div>
      </div>
      <div class="card">
        <h2>Quick Targets</h2>
        <button onclick="run('motion goto M31')">M31</button>
        <button onclick="run('motion goto M42')">M42</button>
        <button onclick="run('motion goto Jupiter')">Jupiter</button>
        <button onclick="run('motion goto Saturn')">Saturn</button>
      </div>
    </div>
  </div>
  <script>
    async function run(cmd) {{
      const onstepUrl = document.getElementById('onstepUrl').value;
      const streamUrl = document.getElementById('streamUrl').value;
      const res = await fetch('/cmd?' + new URLSearchParams({{'cmd': cmd, 'onstep_url': onstepUrl, 'camera_stream_url': streamUrl}}));
      const text = await res.text();
      document.getElementById('out').textContent = text || 'Done.';
    }}
    async function openCamera() {{
      const camera = document.getElementById('cameraSel').value;
      await run('camera open ' + camera);
    }}
    async function applyConfig() {{
      const onstepUrl = document.getElementById('onstepUrl').value;
      const streamUrl = document.getElementById('streamUrl').value;
      const res = await fetch('/cmd?' + new URLSearchParams({{'cmd': 'camera open uvc:0', 'onstep_url': onstepUrl, 'camera_stream_url': streamUrl}}));
      const text = await res.text();
      document.getElementById('out').textContent = 'Config applied.\n' + text;
      document.getElementById('preview').src = streamUrl;
      document.getElementById('preview').style.display = streamUrl ? 'block' : 'none';
      document.getElementById('previewPlaceholder').style.display = streamUrl ? 'none' : 'flex';
    }}
  </script>
</body>
</html>
"""

class Handler(http.server.BaseHTTPRequestHandler):
    def do_GET(self):
        parsed = urllib.parse.urlparse(self.path)
        if parsed.path == '/cmd':
            args = urllib.parse.parse_qs(parsed.query)
            cmd = args.get('cmd', [''])[0]
            onstep_url = args.get('onstep_url', [DEFAULT_ONSTEP_URL])[0]
            camera_stream_url = args.get('camera_stream_url', [DEFAULT_CAMERA_STREAM_URL])[0]
            env = os.environ.copy()
            if onstep_url:
                env['ASDEVLAB_ONSTEP_URL'] = onstep_url
            if camera_stream_url:
                env['ASDEVLAB_CAMERA_STREAM_URL'] = camera_stream_url

            completed = subprocess.run(
                [str(ROOT / 'build' / 'asdevlab_core_app')] + shlex.split(cmd),
                cwd=str(ROOT),
                capture_output=True,
                text=True,
                env=env,
            )
            body = (completed.stdout or '') + (completed.stderr or '')
            self.send_response(200)
            self.send_header('Content-Type', 'text/plain; charset=utf-8')
            self.end_headers()
            self.wfile.write(body.encode())
            return

        self.send_response(200)
        self.send_header('Content-Type', 'text/html; charset=utf-8')
        self.end_headers()
        self.wfile.write(HTML.encode())

if __name__ == '__main__':
    port = int(os.environ.get('ASDEVLAB_WEB_PORT', '8080'))
    print(f'ASDEVLAB web UI listening on http://127.0.0.1:{port}')
    http.server.ThreadingHTTPServer(('127.0.0.1', port), Handler).serve_forever()
