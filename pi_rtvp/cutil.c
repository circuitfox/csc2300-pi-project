#include <math.h>
#include <stdint.h>
#include <stdlib.h>

#include "cutil.h"

static const double r_coeff = 0.2126;
static const double g_coeff = 0.7152;
static const double b_coeff = 0.0722;

uint8_t rgb_to_l(double r, double g, double b)
{
    return (uint8_t)round(r * r_coeff + g * g_coeff + b * b_coeff);
}

uint8_t* convert_to_greyscale(uint8_t* data, int height, int width, int planes)
{
    uint8_t* grey = malloc(width * height);
    int index = 0;
    for (int i = 0; i < height * width * planes; i += planes) {
        grey[index] = rgb_to_l(data[i], data[i+1], data[i+2]);
        index++;
    }
    return grey;
}

int in_bounds(int y, int x, int height, int width, int median)
{
    return y - median >= 0 && x - median >= 0 &&
           y + median <= height - 1 && x + median <= width - 1;
}
