from array import array
import png
import subprocess
import pi_rtvp.camera as camera
import pi_rtvp.core as core

class PNGImage(object):
    def __init__(self, infile, width = 0, height = 0, data = None, info = None):
        try:
            self.file = infile
            if (infile != None):
                image = png.Reader(infile).read()
                self.width = image[0]
                self.height = image[1]
                self.data = list(image[2]) # two-dimensional list[array]
                self.info = image[3]
            else:
                self.width = width
                self.height = height
                self.data = data
                self.info = info
            self.planes = self.info.get("planes", 3) # assume RGB if planes is missing
            self.greyscale = self.info.get("greyscale", False)
        except FileNotFoundError:
            raise

    def __getitem__(self, key):
        return self.data[key]

    def __iter__(self):
        return iter(self.data)

    def __repr__(self):
        return "{}({!r}, {!r}, {!r}, {!r}, {!r})".format(core.fullname(self),
                                                         self.file, self.width,
                                                         self.height, self.data,
                                                         self.info)

    def __str__(self):
        return "{}: {}x{} {}".format(self.file, self.width, self.height, self.info)

    def write(self, outfile):
        writer = png.Writer(self.width, self.height, **self.info)
        if isinstance(outfile, str):
            with open(outfile, "wb") as f:
                writer.write(f, self.data)
        else:
            writer.write(outfile, self.data)

    def to_greyscale(self):
        return convert_to_greyscale(self)

    def is_matrix_in_bounds(self, y, x, size):
        median = int(size / 2)
        return (y - median >= 0 and x - median >= 0 and
                y + median < self.height - 1 and x + median < self.width - 1)

    def is_valid_coord(self, y, x):
        return (y >= 0 and x >= 0 and y < self.height and x < self.width)

    def get_matrix_at(self, y, x, size):
        if not self.is_matrix_in_bounds(y, x, size):
            if not self.is_valid_coord(y, x):
                # TODO: Error handler
                return []
            if is_corner(self, y, x):
                return corner_matrix(self, y, x, size)
            else:
                return edge_matrix(self, y, x, size)
        else:
            data = []
            median = int(size / 2)
            yl = y - median
            xl = x - median
            for i in range(0, size):
                data.append(array("B"))
                for j in range(0, size):
                    data[i].append(self[yl+i][xl+j])
            return data

class PNGImageStream(object):
    def __init__(self):
        self.data = b''

    def write(self, s):
        self.data += s

    def flush(self):
        a = array("B")
        a.frombytes(self.data)
        return PNGImage(a)

def capture_usbcam(usbcamera):
    cap = usbcamera.capture("-", stdout=subprocess.PIPE, stderr=subprocess.DEVNULL)
    data = array('B')
    data.frombytes(cap.stdout)
    return PNGImage(data)

def convert_to_greyscale(image):
    data = list()
    info = image.info.copy();
    r_coeff = 0.2126
    g_coeff = 0.7152
    b_coeff = 0.0722
    for row in image:
        a = array("B")
        for i in range(0, image.width*image.planes, 3):
            grey = round(r_coeff*row[i]+g_coeff*row[i+1]+b_coeff*row[i+2])
            a.append(grey)
        data.append(a)
    info["planes"] = 1
    info["greyscale"] = True
    if "background" in info:
        info["background"] = (255)
    return PNGImage(None, image.width, image.height, data, info)

def is_corner(image, y, x):
    return (x == 0 and y == 0 or
            x == 0 and y == image.height - 1 or
            x == image.width - 1 and y == 0 or
            x == image.width - 1 and y == image.height - 1)

# This stuff is tricky to think about - is there an easier way?
def corner_matrix(image, y, x, size):
    data = []
    median = int(size / 2)
    if x == 0 and y == 0:
        # top left - outs first, extend starts
        for i in range(0, median+1):
            data.append(array("B"))
            data[i].extend([image[y][x]] * median)
            for j in range(0, size-1):
                data[i].append(image[y][x+j])
        for i in range(median+1, size):
            data.append(array("B"))
            data[i].extend([image[y+i][x]] * median)
            for j in range(0, size-1):
                data[i].append(image[y+i][x+j])
    elif x == 0 and y == image.height - 1:
        # bottom left - ins first, extend starts
        for i in range(size, median+1, -1):
            idx = size - i
            imidx = i - 1
            data.append(array("B"))
            data[idx].extend([image[y-imidx][x]] * median)
            for j in range(0, size-1):
                data[idx].append(image[y-imidx][x+j])
        for i in range(median+1, 0, -1):
            idx = size - i
            data.append(array("B"))
            data[idx].extend([image[y][x]] * median)
            for j in range(0, size-1):
                data[idx].append(image[y][x+j])
    elif x == image.width - 1 and y == 0:
        # top right - outs first, extend ends
        for i in range(0, median+1):
            data.append(array("B"))
            for j in range(median, -1, -1):
                data[i].append(image[y][x-j])
            data[i].extend([image[y][x]] * median)
        for i in range (median+1, size):
            data.append(array("B"))
            for j in range(median, -1, -1):
                data[i].append(image[y][x-j])
            data[i].extend([image[y][x]] * median)
    elif x == image.width - 1 and y == image.height - 1:
        # bottom right - ins first, extend ends
        for i in range(size, median+1, -1):
            idx = size - i
            imidx = i - 1
            data.append(array("B"))
            for j in range(median, -1, -1):
                data[idx].append(image[y-imidx][x-j])
            data[idx].extend([image[y-imidx][x]] * median)
        for i in range(median+1, 0, -1):
            idx = size - i
            data.append(array("B"))
            for j in range(median, -1, -1):
                data[idx].append(image[y][x-j])
            data[idx].extend([image[y][x]] * median)
    return data

def edge_matrix(image, y, x, size):
    data = []
    median = int(size / 2)
    if y == 0:
        # top row, outs first
        for i in range(0, median+1):
            data.append(array("B"))
            for j in range(0, size):
                jdx = j - median
                data[i].append(image[y][x+jdx])
        for i in range(median+1, size):
            data.append(array("B"))
            for j in range(0, size):
                jdx = j - median
                data[i].append(image[y+i][x+jdx])
    elif y == image.height - 1:
        # bottom row, outs last
        for i in range(size, median+1, -1):
            idx = size - i
            imidx = i - 1
            data.append(array("B"))
            for j in range(0, size):
                jdx = j - median
                data[idx].append(image[y-imidx][x+jdx])
        for i in range(median+1, 0, -1):
            idx = size - i
            data.append(array("B"))
            for j in range(0, size):
                jdx = j - median
                data[idx].append(image[y][x+jdx])
    elif x == 0:
        # left side, extend first
        for i in range(0, size):
            idx = i - median
            data.append(array("B"))
            data[i].extend([image[y+idx][x]] * median)
            for j in range(0, size-1):
                data[i].append(image[y+idx][x+j])
    elif x == image.width - 1:
        # right side, extend last
        for i in range(0, size):
            idx = i - median
            data.append(array("B"))
            for j in range(median+1, 0, -1):
                jdx = j - 1
                data[i].append(image[y+idx][x-jdx])
            data[i].extend([image[y+idx][x]] * median)
        return data
    return data
