#ifndef PI_RTVP_CONVOLVE_H
#define PI_RTVP_CONVOLVE_H

#include <stdint.h>

enum corner_type {
    CORNER_NONE = 0,
    CORNER_TOPLEFT,
    CORNER_BOTTOMLEFT,
    CORNER_TOPRIGHT,
    CORNER_BOTTOMRIGHT,
};

enum edge_type {
    EDGE_NONE = 0,
    EDGE_TOP,
    EDGE_BOTTOM,
    EDGE_LEFT,
    EDGE_RIGHT,
};

uint8_t
convolve(uint8_t* image_chunk, uint8_t* kernel, int size);

uint8_t
convolve_id(uint8_t* image_chunk, int size);

uint8_t
convolve_gaussian(uint8_t* image_chunk, uint16_t* kernel, int size);

uint8_t
convolve_sobel(uint8_t* image_chunk, int8_t* kernel_x, int8_t* kernel_y, int size);

uint8_t* convolve_image(uint8_t* data, uint8_t* kernel,
                        int height, int width, int size);

uint8_t* convolve_image_id(uint8_t* data, int height, int width, int size);

uint8_t* convolve_image_gaussian(uint8_t* data, uint16_t* kernel,
                                 int height, int width, int size);

uint8_t* convolve_image_sobel(uint8_t* data, int8_t* kernel_x, int8_t* kernel_y,
                              int height, int width, int size);

int is_corner(int y, int x, int height, int width, int median);

uint8_t* get_matrix_at(uint8_t* data, int y, int x, int height, int width, int size);

uint8_t* center_matrix(uint8_t* data, int y, int x, int width, int size);

uint8_t* corner_matrix(uint8_t* data, int y, int x, int width,
                       int size, enum corner_type type);

uint8_t* edge_matrix(uint8_t* data, int y, int x, int width,
                     int size, enum edge_type type);

#endif
