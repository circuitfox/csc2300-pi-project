"""Wrapper interface for the Raspberry Pi GPIO module. This module attempts to
import the real RPi.GPIO module and, if it fails, sets has_gpio to False. When
has_gpio is False, stubs will replace calls to RPi.GPIO functions. This module
exports wrappers for only the functions that this project requires.
"""

has_gpio = True
try:
    import RPi.GPIO as GPIO
except RuntimeError:
    has_gpio = False

if has_gpio:
    BOARD = GPIO.BOARD
    OUT = GPIO.OUT
else:
    BOARD = 0
    OUT = 0

if has_gpio:
    class PWM(GPIO.PWM):
        def __init(self, pin, freq):
            super().__init__(pin, freq)
else:
    class PWM(object):
        def __init__(self, pin, freq):
            self.gpio = pin
            self.freq = freq
            self.dutycycle = 0
    
        def start(self, dc):
            self.dutycycle = dc
            pass
    
        def stop(self):
            pass
    
        def ChangeDutyCycle(self, dc):
            self.dutycycle = dc
            pass

def gpio_wrapper(func, *args):
    if has_gpio:
        func(*args)
    else:
        pass

def setmode(mode):
    gpio_wrapper(GPIO.setmode, mode)

def setup(pin, mode):
    gpio_wrapper(GPIO.setup, pin, mode)

def cleanup():
    gpio_wrapper(GPIO.cleanup)
