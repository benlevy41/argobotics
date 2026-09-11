from utils import ControlState
import asyncio


class Controller:
    COMMAND_MAP = {
        "t": "throttle",
        "y": "yaw",
        "h": "heave",
        "p": "pan",
        "l": "tilt",
    }

    def __init__(self, control_state: ControlState):
        self.control_state = control_state

    def process_command(self, command: str):
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

        return errors

    async def run(self):
        print("control module starting...")
        loop = asyncio.get_running_loop()

        while True:
            command = await loop.run_in_executor(
                None,
                input,
                "> "
            )

            if command.lower() == "quit":
                break

            errors = self.process_command(command)

            for error in errors:
                print(error)

            print(self.control_state)