from os.path import dirname, exists
import subprocess

has_picam = True
try:
    import picamera as camera
except OSError:
    # picamera requires libmmal, which is rpi firmware
    has_picam = False

class Camera(object):
    def __init__(self, path, width, height, fps):
        if exists(dirname(path)):
            self.path = path
        else:
            # /dev/null is guaranteed to exist.
            self.path = "/dev/null"
        self.width = width
        self.height = height
        self.fps = fps

    def __enter__(self):
        raise NotImplementedError("Camera.__enter__")

    def __exit__(self, exc_type, exc_value, exc_tb):
        raise NotImplementedError("Camera.__exit__")

    def __str__(self):
        return "{}: {}x{}@{}fps)".format(self.path, self.width,
                                         self.height, self.fps)

    def __repr__(self):
        return "{}({!r}, {!r}, {!r}, {!r})".format(_fullname(self), self.path,
                                                   self.width, self.height, self.fps)

    def close(self):
        raise NotImplementedError("Camera.close")

    def capture(self, output, format=None, **kwargs):
        raise NotImplementedError("Camera.capture")

class PiCamera(Camera):
    def __init__(self, path, width, height, fps):
        # path should be the callers 'best guess,' there's no guarantee that the
        # picam will actually be at that path, and no way to know for sure!
        if has_picam:
            super().__init__(path, width, height, fps)
            self.cam = picamera.PiCamera(resolution=(width, height), fps=fps)
        else:
            raise RuntimeError("No picamera module is available on your system.")

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_value, exc_tb):
        self.close()

    def __str__(self):
        return "picam@{}: {}x{}@{}fps)".format(self.path, self.width,
                                               self.height, self.fps)

    def __repr__(self):
        return "{}({!r}, {!r}, {!r}, {!r})".format(_fullname(self), self.path,
                                                   self.width, self.height, self.fps)
    def close(self):
        self.cam.close()

    def capture(self, output, format="png", **kwargs):
        self.cam.capture(output, format, **kwargs)

class USBCamera(Camera):
    def __init__(self, path, width, height, fps):
        super().__init__(path, width, height, fps)
        self.args = ["-d", path, "--no-banner", "-r", "{}x{}".format(width, height),
                     "--fps", str(fps)]

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_value, exc_tb):
        self.close()

    def __str__(self):
        return "{}: {}x{}@{}fps), args={}".format(self.path, self.width,
                                                  self.height, self.fps, self.args)
    def __repr__(self):
        return "{}({!r}, {!r}, {!r}, {!r})".format(_fullname(self), self.path,
                                                   self.width, self.height, self.fps)

    def close(self):
        pass

    def capture(self, output, format="png", **kwargs):
        # TODO: Check error codes and throw exceptions
        outargs = ["--{}".format(format), "-1", output]
        return subprocess.run(["fswebcam"] + self.args + outargs, **kwargs)
