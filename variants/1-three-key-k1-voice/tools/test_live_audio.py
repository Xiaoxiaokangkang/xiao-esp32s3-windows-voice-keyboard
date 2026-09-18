import math
import struct
import time

import sounddevice as sd

device_index = next(
    i for i, device in enumerate(sd.query_devices())
    if "Voice Keyboard Audio" in device["name"]
    and device["max_input_channels"] >= 1
    and sd.query_hostapis(device["hostapi"])["name"] == "MME"
)

print("Live microphone check starts in 2 seconds; speak or tap the microphone now.", flush=True)
time.sleep(2)
with sd.RawInputStream(device=device_index, samplerate=48000, channels=1, dtype="int16") as stream:
    raw, overflow = stream.read(48000 * 8)

samples = struct.unpack("<{}h".format(len(raw) // 2), bytes(raw))
peak = max(abs(min(samples)), abs(max(samples)))
rms = math.sqrt(sum(value * value for value in samples) / len(samples))
nonzero = sum(value != 0 for value in samples)
print({"samples": len(samples), "nonzero": nonzero, "min": min(samples), "max": max(samples), "peak": peak, "rms": round(rms, 2), "overflow": overflow})
if peak < 20 or rms < 1:
    raise SystemExit("FAIL: signal level is too low; speech/tap was not detected")
print("PASS: live acoustic signal detected")
