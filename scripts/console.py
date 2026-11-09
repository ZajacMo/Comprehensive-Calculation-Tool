#!/usr/bin/env python3
"""用 headless Edge 抓取页面的 JS 控制台输出与未捕获异常，辅助排查前端问题。

用法：python scripts\console.py <url>
"""
import json
import os
import subprocess
import sys
import tempfile
import time
import urllib.request

EDGE = r"C:\Program Files (x86)\Microsoft\Edge\Application\msedge.exe"


def main():
    url = sys.argv[1]
    # 打开一个带调试端口的临时用户目录，直接读 DevTools 的日志。
    profile = tempfile.mkdtemp(prefix="edge-console-")
    port = 9222
    command = [
        EDGE,
        "--headless=new",
        "--disable-gpu",
        f"--remote-debugging-port={port}",
        f"--user-data-dir={profile}",
        "--enable-logging=stderr",
        "--v=1",
        url,
    ]
    process = subprocess.Popen(command, stdout=subprocess.DEVNULL, stderr=subprocess.PIPE)

    # 等待 DevTools 端点就绪。
    endpoint = None
    for _ in range(40):
        time.sleep(0.25)
        try:
            with urllib.request.urlopen(f"http://127.0.0.1:{port}/json/list", timeout=2) as response:
                targets = json.loads(response.read().decode("utf-8"))
            for target in targets:
                if target.get("type") == "page" and target.get("webSocketDebuggerUrl"):
                    endpoint = target["webSocketDebuggerUrl"]
                    break
        except Exception:
            continue
        if endpoint:
            break

    if not endpoint:
        print("未能连接到 DevTools 端点")
        process.terminate()
        return 1

    # 用 WebSocket 订阅控制台与异常事件。
    import base64
    import socket
    import struct

    # 极简 WebSocket 客户端（只够发一条 Runtime.enable + 收集消息）。
    parsed = endpoint.split("//", 1)[1]
    host_port, path = parsed.split("/", 1)
    host, port_number = host_port.split(":")

    sock = socket.create_connection((host, int(port_number)))
    key = base64.b64encode(os.urandom(16)).decode()
    handshake = (
        f"GET /{path} HTTP/1.1\r\n"
        f"Host: {host_port}\r\n"
        "Upgrade: websocket\r\n"
        "Connection: Upgrade\r\n"
        f"Sec-WebSocket-Key: {key}\r\n"
        "Sec-WebSocket-Version: 13\r\n\r\n"
    )
    sock.sendall(handshake.encode())
    sock.recv(4096)

    def send(text):
        payload = text.encode()
        header = bytearray([0x81])
        length = len(payload)
        if length < 126:
            header.append(0x80 | length)
        elif length < 65536:
            header.append(0x80 | 126)
            header += struct.pack(">H", length)
        else:
            header.append(0x80 | 127)
            header += struct.pack(">Q", length)
        mask = os.urandom(4)
        header += mask
        masked = bytes(b ^ mask[i % 4] for i, b in enumerate(payload))
        sock.sendall(bytes(header) + masked)

    def receive():
        data = sock.recv(65536)
        if len(data) < 2:
            return None
        length = data[1] & 0x7F
        offset = 2
        if length == 126:
            length = struct.unpack(">H", data[2:4])[0]
            offset = 4
        elif length == 127:
            length = struct.unpack(">Q", data[2:10])[0]
            offset = 10
        return data[offset:offset + length].decode("utf-8", errors="replace")

    send(json.dumps({"id": 1, "method": "Runtime.enable"}))
    send(json.dumps({"id": 2, "method": "Log.enable"}))
    send(json.dumps({"id": 3, "method": "Runtime.evaluate",
                      "params": {"expression": "1", "returnByValue": True}}))

    sock.settimeout(6.0)
    collected = []
    try:
        while True:
            message = receive()
            if message is None:
                break
            try:
                event = json.loads(message)
            except json.JSONDecodeError:
                continue
            method = event.get("method", "")
            if method == "Runtime.consoleAPICalled":
                args = event["params"].get("args", [])
                texts = [a.get("value", a.get("description", "")) for a in args]
                collected.append(f"[console.{event['params'].get('type')}] " + " ".join(map(str, texts)))
            elif method == "Runtime.exceptionThrown":
                details = event["params"]["exceptionDetails"]
                text = details.get("exception", {}).get("description") or details.get("text")
                collected.append(f"[异常] {text}")
            elif method == "Log.entryAdded":
                entry = event["params"]["entry"]
                collected.append(f"[log.{entry.get('level')}] {entry.get('text')}")
    except socket.timeout:
        pass
    except Exception as error:
        collected.append(f"[读取失败] {error}")

    sock.close()
    process.terminate()

    if collected:
        print("页面控制台输出：")
        for line in collected:
            print("  " + line)
    else:
        print("控制台无输出（页面没有报错）。")
    return 0


if __name__ == "__main__":
    sys.exit(main())
