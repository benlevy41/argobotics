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

    async def _connect(self):
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
        self.ser.write(cmd.encode('utf-8'))

    def receive_message(self):
        if self.ser.in_waiting:
            return self.ser.readline().decode().strip()
        return None

    async def run(self):
        print("UART module starting...")
        await self._connect()
        print(f"UART connection status connected: {self.connected}")
        while True:
            if (self.ser is None) or (not self.ser.is_open):
                self.connected = False
                print("Connection lost. Attempting to reconnect...")
                await self._connect()
                if not self.connected:
                    await asyncio.sleep(1)
                    continue

            else:
                self.send_command(self.interpreter.packetize(self.control_state))
                message = self.receive_message()
                if message:
                    print(f"Received: {message}")
                await asyncio.sleep(0.02)  
