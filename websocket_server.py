import asyncio
import websockets
from uart_manager import UARTManager
from command_interpreter import CommandInterpreter

uart = UARTManager()

async def handler(websocket, path):
	print("WebSocket connection established")
	async for message in websocket:
		try:
			command = CommandInterpreter.parse(message)
			response = await uart.send_command(command)
			await websocket.send(f"Executed: {command} | Response: {response}")
		except ValueError as ve:
			await websocket.send(f"ve")
		except Exception as e:
			await websocket.send(f"Error: {e}")

async def main():
	async with websockets.serve(handler, "0.0.0.0", 8765):
		print("WebSocket server listening on port 8765")
		await asyncio.Future()

if __name__ == "__main__":
	asyncio.run(main())
