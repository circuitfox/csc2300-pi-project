#ifndef PI_RTVP_CONVOLVE_H
#define PI_RTVP_CONVOLVE_H

#include <stdint.h>

uint8_t
convolve(uint8_t* image_chunk, uint8_t* kernel, int size);

uint8_t
convolve_id(uint8_t* image_chunk, int size);

uint8_t
convolve_gaussian(uint8_t* image_chunk, uint8_t* kernel, int size);

uint8_t
convolve_sobel(uint8_t* image_chunk, int8_t* kernel_x, int8_t* kernel_y, int size);

#endif
