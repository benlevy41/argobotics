import asyncio
import websockets
from functools import partial
from http.server import HTTPServer, SimpleHTTPRequestHandler
from utils import ControlState

class WebServer:
    def __init__(self, ip:str = "0.0.0.0", port:int = 8000):
        self.ip = ip
        self.port = port

    async def run(self):
        self.handler = partial(
            SimpleHTTPRequestHandler,
            directory='web'
        )
        self.server = HTTPServer((self.ip, self.port), self.handler)

        loop = asyncio.get_running_loop()

        print("Starting HTTP server")
        await loop.run_in_executor(
            None,
            self.server.serve_forever
        )

class WebSocket:
    COMMAND_MAP = {
            "t": "throttle",
            "y": "yaw",
            "h": "heave",
            "p": "pan",
            "l": "tilt",
        }
    
    def __init__(self, control_state:ControlState, ip:str="0.0.0.0", port:int=8765):
        self.control_state = control_state
        self.ip = ip
        self.port = port
        self.msg_queue = asyncio.Queue()

    async def handler(self, websocket):
        print("WebSocket connection established")

        async for message in websocket:
            self.process_msg(message)

    def process_msg(self, command:str):
        errors = []
        
        for cmd in command.split(","):
            cmd = cmd.strip()

            try:
                token, value = cmd.split(":")
                attribute = self.COMMAND_MAP[token.strip()]
                value = int(value)

                if not -99 <= value <= 99:
                    raise ValueError("value must be between -99 and 99")

                setattr(self.control_state, attribute, value)

            except ValueError:
                errors.append(f"Invalid command: {cmd}")

            except KeyError:
                errors.append(f"Unknown command: {cmd}")

    async def send_response(self, msg:str):
        pass

    async def run(self):
        async with websockets.serve(self.handler, self.ip, self.port):
            print(f"WebSocket server listening on port {self.port}")
            await asyncio.Future()