import hashlib
import math
import struct
import time

import sounddevice as sd


def find_voice_keyboard():
    devices = sd.query_devices()
    for index, device in enumerate(devices):
        if (
            "Voice Keyboard Audio" in device["name"]
            and device["max_input_channels"] >= 1
            and sd.query_hostapis(device["hostapi"])["name"] == "MME"
        ):
            return index, device
    raise RuntimeError("Voice Keyboard Audio MME capture device not found")


def capture(index):
    with sd.RawInputStream(
        device=index,
        samplerate=48000,
        channels=1,
        dtype="int16",
        blocksize=480,
    ) as stream:
        chunks = []
        overflowed = False
        for _ in range(100):
            data, overflow = stream.read(480)
            chunks.append(bytes(data))
            overflowed |= overflow
    raw = b"".join(chunks)
    samples = struct.unpack("<{}h".format(len(raw) // 2), raw)
    nonzero = sum(value != 0 for value in samples)
    rms = math.sqrt(sum(value * value for value in samples) / len(samples))
    return {
        "bytes": len(raw),
        "nonzero": nonzero,
        "min": min(samples),
        "max": max(samples),
        "rms": round(rms, 2),
        "sha256": hashlib.sha256(raw).hexdigest(),
        "overflow": overflowed,
    }


device_index, device_info = find_voice_keyboard()
print(f"device={device_index} name={device_info['name']}")
first = capture(device_index)
time.sleep(0.5)
second = capture(device_index)
print(f"first={first}")
print(f"second={second}")
if first["nonzero"] == 0:
    raise SystemExit("FAIL: first capture is all zero")
if second["nonzero"] == 0:
    raise SystemExit("FAIL: second capture is all zero")
if first["sha256"] == second["sha256"]:
    raise SystemExit("FAIL: captures are identical")
print("PASS: both independent capture sessions contain fresh non-zero audio")
