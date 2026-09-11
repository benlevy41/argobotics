import serial
import time

print("Opening COM4...")
t = time.perf_counter()

ser = serial.Serial(
    "COM4",
    9600,
    timeout=0.1,
    write_timeout=5
)

print(f"Opened in {time.perf_counter() - t:.3f}s")
print("is_open:", ser.is_open)

cmd = b"+00,+00,+30,+00,+00\n"

print("Writing...")
t = time.perf_counter()

try:
    n = ser.write(cmd)
    print(f"Write returned {n} bytes in {time.perf_counter() - t:.3f}s")

except Exception as e:
    print(f"Write failed after {time.perf_counter() - t:.3f}s")
    print(type(e).__name__, repr(e))

finally:
    ser.close()