import asyncio
from utils import ControlState, UartManager
from controller import Controller


async def main():
    controller = Controller
    state = ControlState()
    uart = UartManager(control_state=state, port='/dev/ttyACM0', baudrate=9600)

    try: 
        await asyncio.gather(
            uart.run()
            controller.run()
        )

    except KeyboardInterrupt:
        print("Exiting...")
        state = ControlState()  # Reset state to neutral

if __name__ == "__main__":
    main() 