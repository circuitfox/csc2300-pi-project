from array import array
from math import sqrt
import numpy as np
from pi_rtvp.util import fullname
from pi_rtvp.png import PNGImage

class ImageKernel(object):
    def __init__(self, name, size, values):
        self.name = name
        self.size = size
        if len(values) != size and len(values[0]) != size:
            raise ValueError("values is not an NxN matrix")
        self.values = values

    def __getitem__(self, key):
        return self.values[key]

    def __len__(self):
        return self.size

    def __repr__(self):
        return "{}({!r}, {!r}, {!r})".format(fullname(self), self.name,
                                             self.size, self.values)

    def __str__(self):
        return "Kernel {!r}, size {}x{}".format(self.name, self.size, self.size)

    def convolve(self, data):
        res = 0
        median = int(self.size / 2)
        if (data == data[median, median]).all():
            return data[median, median]
        res = np.sum(self.values * data)
        return round(res)

    def convolve_image(self, image):
        res_image = PNGImage(None, image.width, image.height,
                             np.empty((image.height,image.width), dtype=np.uint8),
                             image.info)
        for i in range(0, len(image)):
            for j in range (0, len(image[0])):
                res_image[i, j] = self.convolve(image.get_matrix_at(i, j, self.size))
        return res_image

class IdImageKernel(ImageKernel):
    def convolve(self, data):
        median = self.size / 2
        return data[median, median]

class MeanImageKernel(ImageKernel):
    def convolve(self, data):
        res = 0
        median = int(self.size / 2)
        if (data == data[median, median]).all():
            return data[median, median]
        res = np.sum(self.values * data)
        res /= self.size * self.size
        return round(res)

class GaussianImageKernel(ImageKernel):
    def convolve(self, data):
        res = 0
        median = int(self.size / 2)
        if (data == data[median, median]).all():
            return data[median, median]
        res = np.sum(self.values * data)
        res /= np.sum(self.values)
        return round(res)

class SobelImageKernel(ImageKernel):
    def __init__(self, name, size, data_x, data_y):
        super().__init__(name, size, data_x)
        self.data_x = data_x
        self.data_y = data_y

    def __repr__(self):
        return "{}({!r}, {!r}, {!r}, {!r})".format(fullname(self), self.name,
                                                   self.size, self.data_x, self.data_y)

    def convolve(self, data):
        res = 0
        rx = 0
        ry = 0
        median = int(self.size / 2)
        if (data == data[median, median]).all():
            return 0
        rx = np.sum(self.data_x * data)
        ry = np.sum(self.data_y * data)
        res = sqrt(rx * rx + ry * ry) / 8
        return round(res)

class SobelGaussianImageKernel(ImageKernel):
    def __init__(self, name, size, sobel, guassian):
        super().__init__(name, size, sobel.values)
        self.sobel = sobel
        self.gaussian = gaussian

    def __repr__(self):
        return "{}({!r}, {!r}, {!r}, {!r})".format(fullname(self), self.name,
                                                   self.size, self.sobel, self.gaussian)

    def convolve_image(self, image):
        res_image = self.gaussian.convolve_image(image)
        res_image = self.sobel.convolve_image(res_image)
        return res_image

def generate_mean(size):
    if size % 2 == 0:
        return None
    data = []
    for i in range(0, size):
        data.append(array("B"))
        data[i].extend([1] * size)
    return MeanImageKernel("mean", size, np.array(data, dtype=np.uint8))

def generate_gaussian(size):
    if size % 2 == 0:
        return None
    median = int(size / 2)
    data = []
    for i in range(0, median+1):
        data.append(array("H"))
        for j in range(0, median+1):
            data[i].append(2 ** (i+j))
        for j in range(median+1, size):
            jdx = size - j - 1
            data[i].append(2 ** (i+jdx))
    for i in range(median+1, size):
        idx = size - i - 1
        data.append(array("H"))
        for j in range(0, median+1):
            data[i].append(2 ** (idx+j))
        for j in range(median+1, size):
            jdx = size - j - 1
            data[i].append(2 ** (idx+jdx))
    return GaussianImageKernel("gaussian", size, np.array(data, dtype=np.uint16))

id = IdImageKernel("id", 3, np.array([[0, 0, 0], [0, 1, 0], [0, 0, 0]], dtype=np.uint8))

mean = generate_mean(7)

gaussian = generate_gaussian(7)

sobel = SobelImageKernel(
    "sobel", 3,
    np.array([[-1, 0, 1], [-2, 0, 2], [-1, 0, 1]], dtype=np.int8),
    np.array([[-1, -2, -1], [0, 0, 0], [1, 2, 1]], dtype=np.int8)
)

sobel_gaussian = SobelGaussianImageKernel("sobel-gaussian", 3, sobel, generate_gaussian(7))
