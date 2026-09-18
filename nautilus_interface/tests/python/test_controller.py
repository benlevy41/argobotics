import pytest
from nautilus.controller import Controller
from nautilus.utils import ControlState

@pytest.fixture
def control_state():
    state=ControlState(0,0,0,0,0)
    yield state

def test_process_valid_command(control_state):
    valid_command = "t:0,y:30,h:10,p:90,l:-30"
    control = Controller(control_state)

    control.process_command(valid_command)
    assert control_state.throttle == 0
    assert control_state.yaw == 30
    assert control_state.heave == 10
    assert control_state.pan == 90
    assert control_state.tilt == -30

def test_process_part_command(control_state):
    part_command = "t:-30,p:10"
    control = Controller(control_state)

    control.process_command(part_command)
    assert control_state.throttle == -30
    assert control_state.pan == 10
    assert control_state.yaw == 0
    assert control_state.heave == 0
    assert control_state.tilt == 0

def test_process_invalid_command(control_state):
    bad_command = "t:110,y:-10"
    control = Controller(control_state)

    errors = control.process_command(bad_command)
    assert "invalid" in errors[0].lower()
    assert control_state.throttle == 0
    assert control_state.yaw == -10

def test_process_unknown_command(control_state):
    unk_command = "u:37,p:-25"
    control = Controller(control_state)

    errors = control.process_command(unk_command)
    assert "unknown" in errors[0].lower()
    assert control_state.pan == -25

