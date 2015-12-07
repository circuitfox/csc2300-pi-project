#include <math.h>
#include <stdint.h>

#include "convolve.h"

static int all_equal(uint8_t* data, int len)
{
    const uint8_t val = data[0];
    for (int i = 1; i < len; ++i) {
        if (data[i] != val)
            return 0;
    }
    return 1;
}

uint8_t
convolve(uint8_t* image_chunk, uint8_t* kernel, int size)
{
    double res = 0;
    int len = size * size;
    if (all_equal(image_chunk, len)) {
        return image_chunk[0];
    }
    for (int i = 0; i < len; ++i) {
        res += image_chunk[i] * kernel[i];
    }
    res /= size;
    return (uint8_t)round(res);
}

uint8_t
convolve_id(uint8_t* image_chunk, int size)
{
    return image_chunk[size*size/2];
}

uint8_t
convolve_gaussian(uint8_t* image_chunk, uint8_t* kernel, int size)
{
    double res = 0;
    int ksum = 0;
    int len = size * size;
    if (all_equal(image_chunk, len)) {
        return image_chunk[0];
    }
    for (int i = 0; i < len; ++i) {
        res += image_chunk[i] * kernel[i];
        ksum += kernel[i];
    }
    res /= ksum;
    return (uint8_t)round(res);
}

uint8_t
convolve_sobel(uint8_t* image_chunk, int8_t* kernel_x, int8_t* kernel_y, int size)
{
    double res = 0;
    double resx = 0;
    double resy = 0;
    int len = size * size;
    if (all_equal(image_chunk, len)) {
        return 0;
    }
    for (int i = 0; i < len; ++i) {
        resx += image_chunk[i] * kernel_x[i];
        resy += image_chunk[i] * kernel_y[i];
    }
    res = sqrt(resx * resx + resy * resy) / 8;
    return (uint8_t)round(res);
}
