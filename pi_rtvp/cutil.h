#ifndef PI_RTVP_CUTIL_H
#define PI_RTVP_CUTIL_H

#include <stdint.h>

uint8_t rgb_to_l(double r, double g, double b);

uint8_t* convert_to_greyscale(uint8_t* data, int height, int width, int planes);

#endif
