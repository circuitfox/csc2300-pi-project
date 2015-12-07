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
    BCM = GPIO.BCM
    OUT = GPIO.OUT
else:
    BOARD = 0
    BCM = 0
    OUT = 0

if has_gpio:
    class PWM(GPIO.PWM):
        def __init__(self, pin, freq):
            self.gpio = pin
            self.freq = freq
            self.dutycycle = 0
            super().__init__(pin, freq)

        def __str__(self):
            return "PWM at pin {}, freq {}Hz, dutycycle {}".format(self.gpio,
                                                                   self.freq,
                                                                   self.dutycycle)
        def start(self, dc):
            self.dutycycle = dc
            super().ChangeDutyCycle(dc)

        def ChangeDutyCycle(self, dc):
            self.dutycycle = dc
            super().ChangeDutyCycle(dc)
            pass
else:
    class PWM(object):
        def __init__(self, pin, freq):
            self.gpio = pin
            self.freq = freq
            self.dutycycle = 0
    
        def __str__(self):
            return "PWM at pin {}, freq {}Hz, dutycycle {}".format(self.gpio,
                                                                   self.freq,
                                                                   self.dutycycle)

        def start(self, dc):
            self.dutycycle = dc
            pass
    
        def stop(self):
            pass
    
        def ChangeDutyCycle(self, dc):
            self.dutycycle = dc
            pass

def setmode(mode):
    if has_gpio:
        GPIO.setmode(mode)
    else:
        pass

def setup(pin, mode):
    if has_gpio:
        GPIO.setup(pin, mode)
    else:
        pass

def cleanup():
    if has_gpio:
        GPIO.cleanup()
    else:
        pass
