# Flask test app for the Sprint 1 exploration.
# Run with gunicorn so we test a production-style server, not Flask's dev server:
#   gunicorn -w 4 -b 0.0.0.0:8001 app:app
import time
from flask import Flask, request

app = Flask(__name__)

@app.get("/hello")
def hello():
    return "hello from flask\n"

@app.get("/cpu")
def cpu():
    # Busy-loop for ?ms= milliseconds (CPU-bound request)
    ms = int(request.args.get("ms", 20))
    end = time.perf_counter() + ms / 1000
    x = 0
    while time.perf_counter() < end:
        x += 1
    return f"cpu {ms}ms\n"

@app.get("/io")
def io():
    # Sleep for ?ms= milliseconds (stands in for a DB or disk wait)
    ms = int(request.args.get("ms", 20))
    time.sleep(ms / 1000)
    return f"io {ms}ms\n"
