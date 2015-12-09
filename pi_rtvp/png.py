from array import array
import itertools
import numpy as np
import png
import subprocess
import pi_rtvp.util as util
import pi_rtvp.cutil as cutil
import pi_rtvp.matrix as matrix

class PNGImage(object):
    def __init__(self, infile, width = 0, height = 0, data = None, info = None):
        try:
            self.file = infile
            if (infile != None):
                image = png.Reader(infile).asDirect()
                self.width = image[0]
                self.height = image[1]
                self.data = np.vstack(map(np.uint8, image[2]))
                self.info = image[3]
            else:
                self.width = width
                self.height = height
                self.data = data
                self.info = info
            self.planes = self.info.get("planes", 3) # assume RGB if planes is missing
        except FileNotFoundError:
            raise

    def __repr__(self):
        return "{}({!r}, {!r}, {!r}, {!r}, {!r})".format(util.fullname(self),
                                                         self.file, self.width,
                                                         self.height, self.data,
                                                         self.info)

    def __str__(self):
        return "{}: {}x{} {}".format(self.file, self.width, self.height, self.info)

    def write(self, outfile):
        writer = png.Writer(self.width, self.height, **self.info)
        if isinstance(outfile, str):
            with open(outfile, "wb") as f:
                writer.write(f, self.data.reshape(self.height, self.width))
        else:
            writer.write(outfile, self.data.reshape(self.height, self.width))

    def to_greyscale(self):
        return convert_to_greyscale(self)

class PNGImageStream(object):
    def __init__(self):
        self.data = b''
        self.image = None

    def write(self, s):
        self.data += s

    def flush(self):
        a = array("B")
        a.frombytes(self.data)
        self.image = PNGImage(a)

def capture_picam(picamera):
    stream = PNGImageStream()
    picamera.capture(stream, format="png")
    return stream.image

def capture_usbcam(usbcamera):
    cap = usbcamera.capture("-", stdout=subprocess.PIPE, stderr=subprocess.DEVNULL)
    data = array('B')
    data.frombytes(cap.stdout)
    return PNGImage(data)

def convert_to_greyscale(image):
    data = []
    info = image.info.copy()
    data = cutil.convert_to_greyscale(
            image.data, image.height,
            image.width, image.planes).reshape(image.height, image.width)
    info["planes"] = 1
    info["greyscale"] = True
    info["alpha"] = False
    if "background" in info:
        info["background"] = (255)
    return PNGImage(None, image.width, image.height, data, info)
