import wave

with wave.open(r"D:\Trash\logam_0027.wav", "rb") as wf:
    print("Rate:", wf.getframerate())
    print("Channels:", wf.getnchannels())
    print("Frames:", wf.getnframes())