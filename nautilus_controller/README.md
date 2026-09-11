# Nautilus Controller

Nautilus Controller is a PlatformIO firmware project for a Teensy 4.0. It
receives compact control packets over the Teensy's USB serial connection,
converts them into thruster and camera PTZ control values, and updates the motor
outputs. If no valid control packet is received within the keepalive interval,
the controller commands a zero motor state.

## Project layout

```text
src/main.cpp                 Application setup and main control loop
lib/control/                 Packet parsing and control-state calculations
lib/SerialMsgReader/         Newline-delimited serial message buffering
lib/MotorController/         Groups thrusters and camera servos
lib/ManagedMotor/            Shared PWM motor behavior
lib/Thruster/                Thruster-specific update behavior
lib/hardware_config/         Pins, serial speed, timing, and debug settings
scripts/send_control.py      Python USB serial packet sender
test/                        PlatformIO Unity tests
platformio.ini               Teensy 4.0 and Arduino framework configuration
```

PlatformIO uses `platformio.ini` to select the Teensy 4.0 board, the Arduino
framework, Unity tests, and a 9600 baud serial monitor. Libraries under `lib/`
are compiled as project-local modules and can include one another by header.

## Code overview

The runtime path is:

```text
USB Serial
	 |
	 v
main.cpp -> read_serial_msg() -> parse_ctrl()
											 |
											 v
								 ControlState / MotorState
											 |
											 v
								 MotorController
								 /              \
						  Thrusters       Pan / tilt motors
```

`setup()` initializes the motor controller and starts `Serial`. Each pass
through `loop()` reads available USB serial characters. `read_serial_msg()`
buffers characters until a newline, then returns the complete packet. The
control parser expects five signed, two-digit values in this order:

```text
throttle,yaw,heave,pan,tilt
```
for example:

```text
+75,+13,-20,+00,-15
```

`calc_motor_state()` combines throttle and yaw for the left and right
thrusters, applies heave to both depth thrusters, and passes pan and tilt to
the camera motors. `MotorController` sends that state to the individual motor
objects, which update their PWM outputs. Debug output is controlled by the
configuration in `hardware_config.h`.

The host helper accepts a packet with a `CTL:` prefix for convenient command
line use, then removes that prefix before sending because the firmware parser
currently expects only the comma-separated numeric values. A newline is added
as the packet terminator.

## Quick start

1. Open this folder in VS Code with the PlatformIO extension installed. Connect
	the Teensy 4.0 over USB.

2. Build and upload the firmware:

	```powershell
	pio run --target upload
	```

	In VS Code, the PlatformIO Upload command performs the same operation. If
	the port is not selected automatically, choose the Teensy's upload port.

3. Open the serial monitor at 9600 baud. Close the monitor before using the
	Python sender because the USB serial port can only be opened by one program
	at a time:

	```powershell
	pio device monitor --baud 9600
	```

4. Install the sender dependency:

	```powershell
	python -m pip install -r scripts/requirements.txt
	```

5. Close the PlatformIO monitor and send one control packet. Replace `COM4`
	with the Teensy's port:

	```powershell
	python scripts/send_control.py COM4 -p "CTL:+75,+13,-20,+00,-15" --read
	```

	To keep sending packets at 10 Hz, use `--repeat 0`:

	```powershell
	python scripts/send_control.py COM4 -p "CTL:+75,+13,-20,+00,-15" --repeat 0 --interval 0.1 --read
	```

6. Run the firmware tests when needed:

	```powershell
	pio test
	```

The serial monitor and Python sender must not run at the same time because
they compete for the same Teensy USB serial port.

## Hardware configuration

Values are defined in `lib/hardware_config/hardware_config.h`.

| Value | Setting |
| --- | --- |
| `DEPTH_THRUSTER1 = 2` | Depth thruster 1 pin |
| `DEPTH_THRUSTER2 = 3` | Depth thruster 2 pin |
| `RIGHT_THRUSTER = 4` | Right thruster pin |
| `LEFT_THRUSTER = 5` | Left thruster pin |
| `TILT_SERVO = 9` | Camera tilt pin |
| `PAN_SERVO = 10` | Camera pan pin |
| `MESSAGE_LENGTH = 20` | Maximum packet buffer size |
| `KEEPALIVE_INTERVAL = 500` | Zero outputs after 500 ms without a packet |
| `BAUD_RATE = 9600` | USB serial speed |
| `PWM_FREQ = 50` | Motor PWM frequency |
| `ESC_SWITCHING_DELAY = 500` | ESC startup delay in milliseconds |
| `DEBUG_MODE = true` | Print debug messages |
