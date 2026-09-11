#!/usr/bin/env python3
"""Send control packets to a Nautilus Controller over USB serial."""

import argparse
import re
import sys
import time

try:
    import serial
except ImportError:
    serial = None

PACKET_PATTERN = re.compile(r"^CTL:[+-]\d{2}(,[+-]\d{2}){4}$")
BAUD_RATE = 9600


def make_packet(value: str) -> bytes:
    packet = value.removesuffix("\n")
    if not PACKET_PATTERN.fullmatch(packet):
        raise ValueError(
            "packet must look like 'CTL:+75,+13,-20,+00,-15' "
            "with five signed two-digit values"
        )
    packet = packet.removeprefix("CTL:")
    return f"{packet}\n".encode("ascii")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("port", help="Teensy USB serial port, for example COM5")
    parser.add_argument(
        "--packet",
        "-p",
        dest="packet",
        nargs="?",
        default="CTL:+00,+00,+00,+00,+00",
        help="CTL-prefixed packet with five signed two-digit values (default: %(default)s)",
    )
    parser.add_argument(
        "--repeat",
        type=int,
        default=1,
        help="number of packets to send; use 0 to send continuously",
    )
    parser.add_argument(
        "--interval",
        type=float,
        default=0.1,
        help="seconds between repeated packets (default: %(default)s)",
    )
    parser.add_argument(
        "--read",
        action="store_true",
        help="print text returned by the Teensy",
    )
    args = parser.parse_args()

    if args.repeat < 0:
        parser.error("--repeat must be zero or greater")
    if args.interval < 0:
        parser.error("--interval must be zero or greater")

    try:
        payload = make_packet(args.packet)
    except ValueError as error:
        parser.error(str(error))

    if serial is None or not hasattr(serial, "Serial"):
        print(
            "pyserial is required. Install it with: python -m pip install pyserial",
            file=sys.stderr,
        )
        return 1

    try:
        with serial.Serial(args.port, BAUD_RATE, timeout=0.1) as connection:
            print(f"Sending {payload.decode().rstrip()} to {args.port} at {BAUD_RATE} baud")
            sent = 0
            while args.repeat == 0 or sent < args.repeat:
                connection.write(payload)
                connection.flush()
                sent += 1

                if args.read:
                    response = connection.read_all().decode("ascii", errors="replace")
                    if response:
                        print(response, end="")

                if args.repeat == 0 or sent < args.repeat:
                    time.sleep(args.interval)
    except serial.SerialException as error:
        print(f"Could not open or use {args.port}: {error}", file=sys.stderr)
        return 1
    except KeyboardInterrupt:
        print("\nStopped.")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
