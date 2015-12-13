from array import array
import itertools
import numpy as np
import subprocess
import pi_rtvp.util as util
import pi_rtvp.cutil as cutil
import pi_rtvp.image as im
import pi_rtvp.matrix as matrix

class PNGImage(object):
    def __init__(self, infile, width = 0, height = 0, data = None, info = None):
        try:
            self.file = infile
            if (infile != None):
                if isinstance(infile, str):
                    image = im.read_png(infile)
                else:
                    data = np.array(infile, dtype=np.uint8)
                    image = im.read_png_data(np.array(infile, dtype=np.uint8))
                self.width = image[0]
                self.height = image[1]
                self.data = image[2]
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
        if isinstance(outfile, str):
            im.write_png(outfile, self.width, self.height, self.data, self.info)
        else:
            im.write_png_io(outfile, self.width, self.height, self.data, self.info)

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
    data = array("B")
    data.frombytes(cap.stdout)
    return PNGImage(data)

def convert_to_greyscale(image):
    data = []
    info = image.info.copy()
    data = cutil.convert_to_greyscale(
            image.data, image.height,
            image.width, image.planes).reshape(image.height, image.width)
    info["planes"] = 1
    info["color_type"] = 0
    return PNGImage(None, image.width, image.height, data, info)
