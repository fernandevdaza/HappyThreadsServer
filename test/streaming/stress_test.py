import asyncio
import re
import time
from dataclasses import dataclass


@dataclass
class Stats:
    ok_segments: int = 0
    fail_segments: int = 0
    bytes: int = 0
    playlist_ok: int = 0
    playlist_fail: int = 0


async def http_get(host: str, port: int, path: str, timeout_s: float = 8.0) -> bytes:
    reader, writer = await asyncio.wait_for(
        asyncio.open_connection(host, port), timeout=timeout_s
    )
    req = (
        f"GET {path} HTTP/1.1\r\n"
        f"Host: {host}\r\n"
        f"Connection: close\r\n"
        f"User-Agent: hls_http_stress\r\n"
        f"\r\n"
    ).encode()
    writer.write(req)
    await writer.drain()

    data = b""
    while True:
        chunk = await reader.read(65536)
        if not chunk:
            break
        data += chunk

    writer.close()
    await writer.wait_closed()

    i = data.find(b"\r\n\r\n")
    if i == -1:
        return b""
    head = data[:i].decode("latin1", "ignore")
    if " 200 " not in head and " 206 " not in head:
        raise RuntimeError(head.splitlines()[0] if head else "bad response")
    return data[i + 4 :]


def parse_segments(m3u8_text: str) -> list[str]:
    segs = []
    for line in m3u8_text.splitlines():
        line = line.strip()
        if not line or line.startswith("#"):
            continue
        segs.append(line)
    return segs


def join_path(base_dir: str, ref: str) -> str:
    if ref.startswith("http://") or ref.startswith("https://"):
        raise ValueError("no externo")
    if ref.startswith("/"):
        return ref
    if not base_dir.endswith("/"):
        base_dir += "/"
    return base_dir + ref


async def hls_client(
    client_id: int,
    host: str,
    port: int,
    playlist_path: str,
    segments_per_session: int,
    iterations: int,
    stats: Stats,
):
    base_dir = playlist_path.rsplit("/", 1)[0] if "/" in playlist_path else ""
    if not base_dir.startswith("/"):
        base_dir = "/" + base_dir if base_dir else ""

    for _ in range(iterations):
        try:
            pl_bytes = await http_get(host, port, playlist_path)
            stats.playlist_ok += 1
        except Exception:
            stats.playlist_fail += 1
            await asyncio.sleep(0.2)
            continue

        try:
            segs = parse_segments(pl_bytes.decode("utf-8", "ignore"))
        except Exception:
            await asyncio.sleep(0.2)
            continue

        for seg in segs[:segments_per_session]:
            seg_path = join_path(base_dir, seg)
            try:
                b = await http_get(host, port, seg_path)
                stats.ok_segments += 1
                stats.bytes += len(b)
            except Exception:
                stats.fail_segments += 1


async def main():
    import argparse

    ap = argparse.ArgumentParser()
    ap.add_argument("--host", default="127.0.0.1")
    ap.add_argument("--port", type=int, default=8000)
    ap.add_argument(
        "--playlist", required=True, help="ej: /hls/ciberseguridad/index.m3u8"
    )
    ap.add_argument("--clients", type=int, default=200)
    ap.add_argument("--iterations", type=int, default=50)
    ap.add_argument("--segments", type=int, default=10)
    ap.add_argument("--concurrency", type=int, default=200)
    args = ap.parse_args()

    stats = Stats()
    sem = asyncio.Semaphore(args.concurrency)

    async def guarded(i: int):
        async with sem:
            await hls_client(
                i,
                args.host,
                args.port,
                args.playlist,
                args.segments,
                args.iterations,
                stats,
            )

    t0 = time.time()
    await asyncio.gather(*[guarded(i + 1) for i in range(args.clients)])
    dt = time.time() - t0

    mb = stats.bytes / (1024 * 1024)
    print("clients:", args.clients)
    print("playlist_ok:", stats.playlist_ok, "playlist_fail:", stats.playlist_fail)
    print("segments_ok:", stats.ok_segments, "segments_fail:", stats.fail_segments)
    print(f"downloaded: {mb:.2f} MiB in {dt:.2f}s  -> {mb/dt:.2f} MiB/s")


if __name__ == "__main__":
    asyncio.run(main())
