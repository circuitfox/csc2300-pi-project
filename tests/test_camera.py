import unittest
import pi_rtvp.camera as camera

class PiCameraTestCase(unittest.TestCase):
    def test_init_exception_no_picam(self):
        if not camera.has_picam:
            with self.assertRaises(RuntimeError):
                camera.PiCamera("/dev/null", 0, 0, 0)
