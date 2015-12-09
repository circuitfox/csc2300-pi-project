"""Interface to control the yaw and pitch motors."""

def angle_to_dutycycle(angle):
    """Converts an angle to a duty cycle in the range of 0 to 100.
    This will clamp its results into this proper range.
    """
    cycle = ((angle / 50) + 1) * 5
    if (cycle < 0):
        cycle = 0
    elif (cycle > 100):
        cycle = 100
    return cycle

def dutycycle_to_angle(dc):
    """Converts a duty cycle value in the range of 0 to 100 into
    an angular value between -180 and 3420
    """
    if (dc < 0):
        dc = 0
    elif (dc > 100):
        dc = 100
    return (dc / 5 - 1) * 50

def set_pitch(state, angle):
    if angle < 0:
        angle = 0
    elif angle > 180:
        angle = 180
    state.pitch_motor.ChangeDutyCycle(angle_to_dutycycle(angle))
  
def set_yaw(state, angle):
    if angle < 0:
        angle = 0
    elif angle > 180:
        angle = 180
    state.yaw_motor.ChangeDutyCycle(angle_to_dutycycle(angle))
