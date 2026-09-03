import sys
import urllib.request

if __name__ == '__main__':
    if len(sys.argv) < 3:
        sys.exit(1)

    base = sys.argv[1]
    path = sys.argv[2]
    query = sys.argv[3] if len(sys.argv) > 3 else ''
    url = base + path
    if query:
        url = f"{url}?{query}"

    try:
        with urllib.request.urlopen(url, timeout=3) as response:
            body = response.read().decode('utf-8', 'ignore').strip()
            print(body)
    except Exception as exc:
        print(f"proxy_error:{exc}")
        sys.exit(1)
