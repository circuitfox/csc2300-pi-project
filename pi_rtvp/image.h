#ifndef PI_RTVP_IMAGE_H
#define PI_RTVP_IMAGE_H

#include <stdint.h>
#include <stdio.h>

struct png_data {
    uint32_t width;
    uint32_t height;
    uint8_t* data;
    int bit_depth;
    int color_type;
    int planes;
};

struct png_buffer {
    uint8_t* buffer;
    uint32_t position;
};

struct png_data* read_png(const char* file);

struct png_data* read_png_buffer(uint8_t* image);

void write_png(const char* outfile, struct png_data* data);

void write_png_fp(FILE* write_png_fp, struct png_data* data);

struct png_buffer* write_png_buffer(struct png_data* png);

#endif
