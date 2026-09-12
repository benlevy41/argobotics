import asyncio
from utils import ControlState, UartManager, PacketBuilder
from controller import Controller
from web import WebSocket


async def main():
    state = ControlState()
    controller = Controller(state)
    websocket = WebSocket(state)
    uart = UartManager(control_state=state, port='COM4', baudrate=9600)

    try: 
        print("Welcome to the ROV controller interface!")

        await asyncio.gather(
            uart.run(),
            controller.run(),
            web
        )

    except KeyboardInterrupt:
        print("Exiting...")
        state = ControlState()  # Reset state to neutral

if __name__ == "__main__":
    asyncio.run(main()) 