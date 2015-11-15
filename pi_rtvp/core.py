"""This module contains core classes such as the program's state, and helper
functions that will be used throughout the rest of the project.
"""

import pi_rtvp.gpio as GPIO
from pi_rtvp.motor import angle_to_dutycycle

class ProgramState(object):
    """Container for global state variables, functions that intend to use these
    should take the state as their first parameter
    """

    # TODO: kernel param should be a Kernel object, not a string
    def __init__(self, yaw_motor, pitch_motor,
                 picam_path="/dev/video0", usbcam_path="/dev/video1", kernel="mean"):
        self.yaw = 0
        self.pitch = 0
        self.output = "/dev/null"
        self.yaw_motor = yaw_motor
        self.pitch_motor = pitch_motor
        self.picam_path = picam_path
        self.usbcam_path = usbcam_path
        self.kernel = kernel

    def __repr__(self):
       return "ProgramState({!r}, {!r}, {!r}, {!r}, {!r})".format(
               self.yaw_motor, self.pitch_motor,
               self.picam_path, self.usbcam_path,
               self.kernel)

    def __str__(self):
        return ("yaw={}\n"
                "pitch={}\n"
                "yaw_motor={}\n"
                "pitch_motor={}\n"
                "picam_path={}\n"
                "usbcam_path={}\n"
                "output={}\n"
                "kernel={}").format(self.yaw, self.pitch, self.yaw_motor,
                                    self.pitch_motor, self.picam_path,
                                    self.usbcam_path, self.output, self.kernel)

def setup(yaw_motor, pitch_motor, freq=100):
    """Attempt to setup the Raspberry Pi and PWM outputs.
    Returns a state object to be used throughout the program.
    """
    GPIO.setmode(GPIO.BOARD)
    GPIO.setup(yaw_motor, GPIO.OUT)
    GPIO.setup(pitch_motor, GPIO.OUT)
    yaw_pwm = GPIO.PWM(yaw_motor, 100)
    pitch_pwm = GPIO.PWM(pitch_motor, 100)
    state = ProgramState(yaw_pwm, pitch_pwm)
    state.yaw_motor.start(angle_to_dutycycle(state.yaw))
    state.pitch_motor.start(angle_to_dutycycle(state.pitch))
    return state

def cleanup(state):
    """Clean up the state of the Raspberry Pi before program exit."""
    state.yaw_motor.stop()
    state.pitch_motor.stop()
    GPIO.cleanup()

def fullname(o):
    return o.__module__ + "." + o.__class__.__qualname__
