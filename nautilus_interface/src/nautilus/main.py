import asyncio
import argparse
import os
import signal
from nautilus_interface.src.nautilus.utils import ControlState, UartManager
from nautilus_interface.src.nautilus.controller import Controller
from nautilus_interface.src.nautilus.web_server import WebSocket, WebServer


async def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--disable-uart",
        action="store_true",
        help="disables uart manager for debugging"
    )
    parser.add_argument(
        "--disable-console",
        action="store_true",
        help="disables interactive console control"
    )
    parser.add_argument(
        "--uart-port",
        default=os.environ.get("NAUTILUS_UART_PORT", "/dev/ttyACM0"),
        help="UART device (default: %(default)s)"
    )
    parser.add_argument(
        "--baudrate",
        type=int,
        default=int(os.environ.get("NAUTILUS_BAUDRATE", "9600")),
        help="UART baud rate (default: %(default)s)"
    )
    parser.add_argument(
        "--http-port",
        type=int,
        default=int(os.environ.get("NAUTILUS_HTTP_PORT", "8000")),
        help="HTTP server port (default: %(default)s)"
    )
    parser.add_argument(
        "--websocket-port",
        type=int,
        default=int(os.environ.get("NAUTILUS_WS_PORT", "8765")),
        help="WebSocket server port (default: %(default)s)"
    )
    args = parser.parse_args()

    state = ControlState()
    controller = Controller(state)
    websocket = WebSocket(state, port=args.websocket_port)
    http_server = WebServer(port=args.http_port)
    uart = UartManager(
        control_state=state,
        port=args.uart_port,
        baudrate=args.baudrate
    )
    shutdown_event = asyncio.Event()

    def request_shutdown():
        shutdown_event.set()

    loop = asyncio.get_running_loop()
    for signum in (signal.SIGINT, signal.SIGTERM):
        try:
            loop.add_signal_handler(signum, request_shutdown)
        except (NotImplementedError, RuntimeError):
            pass

    tasks = [
        asyncio.create_task(websocket.run(), name="websocket"),
        asyncio.create_task(http_server.run(), name="http"),
    ]
    if not args.disable_uart:
        tasks.append(asyncio.create_task(
            uart.run(shutdown_event),
            name="uart"
        ))
    if not args.disable_console:
        tasks.append(asyncio.create_task(controller.run(), name="console"))
    shutdown_task = asyncio.create_task(
        shutdown_event.wait(),
        name="shutdown"
    )

    try:
        print("Welcome to the ROV controller interface!")
        done, _ = await asyncio.wait(
            [*tasks, shutdown_task],
            return_when=asyncio.FIRST_COMPLETED
        )
        for task in done:
            if task is shutdown_task:
                continue
            exception = task.exception()
            if exception is not None:
                raise exception
    finally:
        shutdown_event.set()
        state.throttle = 0
        state.yaw = 0
        state.heave = 0
        state.pan = 0
        state.tilt = 0
        await uart.close()

        for task in tasks:
            if not task.done():
                task.cancel()
        if not shutdown_task.done():
            shutdown_task.cancel()
        await asyncio.gather(
            *tasks,
            shutdown_task,
            return_exceptions=True
        )
        print("Exiting...")

if __name__ == "__main__":
    asyncio.run(main()) 