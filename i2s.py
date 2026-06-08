import serial
import wave
import time
import numpy as np

PORT = "COM3"
BAUD = 921600

DURATION = 50
OUTPUT_FILE = "Logam1.wav"

ser = serial.Serial(PORT, BAUD)
time.sleep(2)

print("Recording...")

raw = b""

start = time.time()

while time.time() - start < DURATION:
    raw += ser.read(ser.in_waiting or 1)

ser.close()

if len(raw) % 2:
    raw = raw[:-1]

samples = np.frombuffer(raw, dtype=np.int16)

print("Samples:", len(samples))

wf = wave.open(OUTPUT_FILE, "wb")

wf.setnchannels(1)
wf.setsampwidth(2)
wf.setframerate(16000)

wf.writeframes(samples.tobytes())
wf.close()

print("Saved:", OUTPUT_FILE)