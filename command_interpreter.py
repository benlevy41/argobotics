class CommandInterpreter:
	VALID_COMMANDS = {
		'w':'f 1',
		'a':'l 1',
		's':'r 1',
		'd':'f-1',
		'x':'x',
		'=':'u 2',
		'-':'u-1'
	}

	@staticmethod
	def parse(input_str):
		key = input_str.lower().strip()
		if key in CommandInterpreter.VALID_COMMANDS:
			return CommandInterpreter.VALID_COMMANDS[key]
		raise ValueError(f"Invalid input: {input_str}")
