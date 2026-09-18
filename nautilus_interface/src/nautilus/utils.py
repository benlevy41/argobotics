from dataclasses import dataclass
import serial
import asyncio

@dataclass
class ControlState:
    throttle: int = 0
    yaw: int = 0
    heave: int = 0
    pan: int = 0
    tilt: int = 0

class PacketBuilder:
    def __init__(self):
        pass

    @staticmethod
    def packetize(state: ControlState) -> str:
        values = (
            state.throttle,
            state.yaw,
            state.heave,
            state.pan,
            state.tilt,
        )
        if any(value < -99 or value > 99 for value in values):
            raise ValueError("control values must be between -99 and 99")
        return f"{','.join(f'{value:+03d}' for value in values)}\n"
        
    
class UartManager:
    def __init__(self, control_state: ControlState, port='/dev/ttyACM0', baudrate=9600):
        self.ser = None
        self.port = port
        self.baudrate = baudrate
        self.control_state = control_state
        self.interpreter = PacketBuilder()
        self.connected = False
        self._closed = False

    async def _connect(self):
        if self._closed:
            return

        try:
            self.ser = serial.Serial(
                self.port,
                self.baudrate,
                timeout=0.1,
                write_timeout=0.1
            )

            self.connected = self.ser.is_open

            if self.connected:
                print("Serial port opened")
                await asyncio.sleep(1)
                print("ready to transmit")

        except serial.SerialException as e:
            print(f"UART connection failed: {e}")
            self.ser = None
            self.connected = False

    def send_command(self, cmd):
        if not self.ser or not self.ser.is_open:
            return
        self.ser.write(str(cmd).encode('utf-8'))

    def receive_message(self):
        if self.ser and self.ser.is_open and self.ser.in_waiting:
            return self.ser.readline().decode().strip()
        return None

    async def close(self):
        self._closed = True
        self.connected = False
        if self.ser:
            self.ser.close()
            self.ser = None

    async def _wait_or_stop(self, stop_event:asyncio.Event, timeout): # this function does two  separate things which makes the purpose unclear - suggest we separate the wait logic from the stop logic
        if stop_event is None:
            await asyncio.sleep(timeout)
            return False

        try:
            await asyncio.wait_for(stop_event.wait(), timeout)

        except asyncio.TimeoutError:
            return False
        
        return True

    async def run(self, stop_event:asyncio.Event=None):
        if not ((type(stop_event) == asyncio.locks.Event) or (stop_event is None)):
            raise ValueError("stop_event must be asyncio Event or None!")

        print("UART module starting...")
        await self._connect()
        print(f"UART connection status connected: {self.connected}")
        
        while not self._closed and not (stop_event and stop_event.is_set()): 
            if (self.ser is None) or (not self.ser.is_open):
                self.connected = False
                print("Connection lost. Attempting to reconnect...")
                await self._connect()
                if not self.connected:
                    if await self._wait_or_stop(stop_event, 1): # check for stop event, then wait and continue
                        break
                    continue

            else:
                try:
                    self.send_command(self.interpreter.packetize(self.control_state))
                    message = self.receive_message()
                    if message:
                        print(f"Received: {message}")

                except (serial.SerialException, OSError) as error:
                    print(f"UART connection lost: {error}")
                    await self.close()
                    self._closed = False

                except ValueError as e:
                    print(f"Control state contained invalid values: [{self.control_state.throttle}, {self.control_state.heave}, {self.control_state.yaw}, {self.control_state.pan}, {self.control_state.tilt}]")
                    print(f"Error: {e}")
                    await self.close()
                    self._closed = False

                except TypeError as e:
                    print(f"Control state value was wrong type: [{type(self.control_state.throttle)}, {type(self.control_state.heave)}, {type(self.control_state.yaw)}, {type(self.control_state.pan)}, {type(self.control_state.tilt)}]")
                    print(f"Error: {e}")
                    await self.close()
                    self._closed = False
                
                if await self._wait_or_stop(stop_event, 0.02):
                    break

        await self.close()
