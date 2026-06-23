import serial
import asyncio

class UARTManager:
	def __init__(self, port='/dev/ttyACM0', baudrate=9600):
		self.ser = serial.Serial(port, baudrate, timeout=1)

	async def send_command(self,cmd):
		if not self.ser.is_open:
			self.ser.open()
		self.ser.write(cmd.encode('utf-8'))
		await asyncio.sleep(0.05)
		if self.ser.in_waiting:
			return self.ser.readline().decode().strip()
		return "sent"
