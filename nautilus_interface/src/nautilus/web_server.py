import asyncio
import websockets
from functools import partial
from http.server import HTTPServer, SimpleHTTPRequestHandler
from nautilus.utils import ControlState
from nautilus.controller import Controller

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

class WebSocket(Controller):

    def __init__(self, control_state:ControlState, ip:str="0.0.0.0", port:int=8765):
        self.super().__init__(control_state)
        self.ip = ip
        self.port = port
        self.msg_queue = asyncio.Queue()

    async def handler(self, websocket):
        print("WebSocket connection established")

        async for message in websocket:
            self.process_msg(message)

    def process_msg(self, command:str):
        errors = super().process_command(command) # eventually we may handle other types of messages

    async def send_response(self, msg:str): # future implementation for telemetry, etc
        pass

    async def run(self):
        async with websockets.serve(self.handler, self.ip, self.port):
            print(f"WebSocket server listening on port {self.port}")
            await asyncio.Future()