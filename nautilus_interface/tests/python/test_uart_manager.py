import asyncio
import pytest
from unittest.mock import MagicMock, patch
from nautilus.utils import UartManager, ControlState, PacketBuilder


@pytest.fixture
def mock_serial():
    serial_instance = MagicMock()
    serial_instance.is_open = True
    serial_instance.in_waiting = False

    with patch(
        "nautilus.utils.serial.Serial",
        return_value=serial_instance,
    ) as mock:
        yield mock

@pytest.fixture
def control_state():
    state = ControlState(0, 0, 0, 0, 0)
    yield state

def test_uart_connect(control_state, mock_serial, capsys):
    uart = UartManager(control_state, "", 9600)
    asyncio.run(uart._connect())
    cap = capsys.readouterr()
    assert mock_serial.return_value.is_open is True
    assert uart.connected is True
    assert "serial port opened" in cap.out.lower()

def test_uart_send_command(control_state, mock_serial):
    uart = UartManager(control_state, "", 9600)
    msg = "test"
    uart.send_command(msg)
    mock_serial.assert_not_called()
    asyncio.run(uart._connect())
    uart.send_command(msg)
    mock_serial.return_value.write.assert_called_once_with(msg.encode("utf-8"))
    int_msg = 1080
    uart.send_command(int_msg)
    mock_serial.return_value.write.assert_called_with(str(int_msg).encode("utf-8"))

def test_uart_receive_message(control_state, mock_serial):
    mock_serial.return_value.readline.return_value = b"test\n"
    uart = UartManager(control_state, "", 9600)
    asyncio.run(uart._connect())
    response = uart.receive_message()
    assert response is None
    mock_serial.return_value.in_waiting = True
    response = uart.receive_message()
    assert response == "test"

@pytest.mark.asyncio
async def test_uart_run(control_state, mock_serial, capsys):
    packet = PacketBuilder.packetize(control_state)
    mock_serial.return_value.readline.return_value = b"test\n"
    mock_serial.return_value.in_waiting = True
    uart = UartManager(control_state, "", 9600)

    stop_event = asyncio.Event()
    run_task = asyncio.create_task(uart.run(stop_event))

    await asyncio.sleep(1.05)
    stop_event.set()
    await run_task  
    cap = capsys.readouterr()

    mock_serial.return_value.write.assert_called_with(packet.encode("utf-8"))
    assert "test" in cap.out.lower()
    assert uart.ser is None     #properly shutdown


@pytest.mark.asyncio
async def test_uart_run_lost_connection(control_state, mock_serial):
    first_connect = MagicMock()
    first_connect.is_open = True
    second_connect = MagicMock()
    second_connect.is_open = False

    mock_serial.side_effect = [
        first_connect,
        second_connect
    ]

    uart = UartManager(control_state, "", 9600)
    stop_event = asyncio.Event()
    run_task = asyncio.create_task(uart.run(stop_event))

    await asyncio.sleep(1.05)
    first_connect.is_open = False
    await asyncio.sleep(0.05)
    assert mock_serial.call_count >= 2

    stop_event.set()
    await run_task

