import serial
import wave
import time
import numpy as np
from scipy import signal

# ====== CONFIG ======
PORT = "COM3"
BAUD = 921600
DURATION = 10
OUTPUT_FILE = "D:/Trash/Plastik_0044.wav"
TARGET_RATE = 16000

# ====== INIT SERIAL ======
ser = serial.Serial(PORT, BAUD)
time.sleep(2)

print("Recording...")
raw = b""

# ====== RECORD ======
start_time = time.time()
while time.time() - start_time < DURATION:
    raw += ser.read(ser.in_waiting or 1)

ser.close()

if len(raw) % 2 != 0:
    raw = raw[:-1]

# ====== HITUNG SAMPLE RATE AKTUAL ======
actual_samples = len(raw) // 2
actual_rate = actual_samples / DURATION
print(f"Actual sample rate: {actual_rate:.0f} Hz")
print(f"Total samples: {actual_samples}")

# ====== KONVERSI + KOREKSI ======
samples = np.frombuffer(raw, dtype=np.uint16).astype(np.float32)
samples = np.clip(samples, 0, 4095)

# Hapus DC offset (idle voltage MAX9814)
samples -= np.mean(samples)

# Normalisasi
max_val = np.max(np.abs(samples))
if max_val > 0:
    samples = samples / max_val

# ====== RESAMPLE ke TARGET_RATE ======
resampled = signal.resample_poly(
    samples,
    TARGET_RATE,
    int(actual_rate)
)

# Konversi ke int16
resampled = np.clip(resampled * 32767, -32768, 32767).astype(np.int16)

# ====== SAVE WAV ======
print("Saving WAV...")
wf = wave.open(OUTPUT_FILE, 'w')
wf.setnchannels(1)
wf.setsampwidth(2)
wf.setframerate(TARGET_RATE)
wf.writeframes(resampled.tobytes())
wf.close()

print(f"Done! File saved: {OUTPUT_FILE}")
print(f"Resampled: {actual_rate:.0f} Hz → {TARGET_RATE} Hz")