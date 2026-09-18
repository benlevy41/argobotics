import pytest
from nautilus.utils import PacketBuilder, ControlState


def test_packetize_valid_packet():
    valid_packet = ControlState(0,0,0,0,0)
    packet = PacketBuilder.packetize(valid_packet)
    assert packet == "+00,+00,+00,+00,+00\n"

def test_packetize_invalid_value():                     # raises exception handled by UartManager
    bad_value = ControlState(-140,0,0,0,0)
    with pytest.raises(ValueError):
        packet = PacketBuilder.packetize(bad_value)

def test_packetize_bad_packet():                        # raises exception handled by UartManager
    bad_packet = ControlState("0",0,0,0,0)
    with pytest.raises(TypeError):
        packet = PacketBuilder.packetize(bad_packet)