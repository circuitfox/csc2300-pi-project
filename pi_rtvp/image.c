#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <png.h>

#include "image.h"

static void buffer_read(png_structp png_ptr, png_bytep data, png_size_t len)
{
    png_voidp io_ptr = png_get_io_ptr(png_ptr);
    if (!io_ptr) {
        return;
    }
    struct png_buffer* dst = (struct png_buffer*)io_ptr;
    memcpy(data, dst->buffer+dst->position, len);
    dst->position += len;
}

static void buffer_write(png_structp png_ptr, png_bytep data, png_size_t len)
{
    png_voidp io_ptr = png_get_io_ptr(png_ptr);
    if (!io_ptr) {
        return;
    }
    struct png_buffer* src = (struct png_buffer*)io_ptr;
    size_t size = src->position + len;
    if(src->buffer) {
        src->buffer = realloc(src->buffer, size);
    } else {
        src->buffer = malloc(size);
    }
    if (!src) {
        png_error(png_ptr, "Write failed");
    }
    memcpy(src->buffer+src->position, data, len);
    src->position += len;
}

static void fakeflush(png_structp png_ptr)
{
}

static void build_png_data(png_structp png_ptr, png_infop info_ptr, struct png_data* data);

struct png_data* read_png(const char* file)
{
    FILE* filep;
    struct png_data* data;
    png_structp png_ptr;
    png_infop info_ptr;
    if (!(filep = fopen(file, "r"))) {
        return NULL;
    }
    if (!(data = malloc(sizeof(*data)))) {
        return NULL;
    }
    uint8_t sig[8];
    fread(sig, 1, 8, filep);
    // png_sig_cmp returns 0 on success
    if (png_sig_cmp(sig, 0, 8)) {
        free(data);
        fclose(filep);
        return NULL;
    }

    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr) {
        free(data);
        fclose(filep);
        return NULL;
    }

    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        png_destroy_read_struct(&png_ptr, NULL, NULL);
        free(data);
        fclose(filep);
        return NULL;
    }

    if (setjmp(png_jmpbuf(png_ptr))) {
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        free(data);
        fclose(filep);
        return NULL;
    }

    png_init_io(png_ptr, filep);
    png_set_sig_bytes(png_ptr, 8);
    png_read_info(png_ptr, info_ptr);
    build_png_data(png_ptr, info_ptr, data);
    if (!data) {
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        fclose(filep);
        return NULL;
    }
    return data;
}

struct png_data* read_png_buffer(uint8_t* image)
{
    struct png_data* data;
    png_structp png_ptr;
    png_infop info_ptr;
    if (!(data = malloc(sizeof(*data)))) {
        return NULL;
    }
    uint8_t sig[8];
    memcpy(sig, image, 8);
    // png_sig_cmp returns 0 on success
    if (png_sig_cmp(sig, 0, 8)) {
        free(data);
        return NULL;
    }

    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr) {
        free(data);
        return NULL;
    }

    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        png_destroy_read_struct(&png_ptr, NULL, NULL);
        free(data);
        return NULL;
    }

    if (setjmp(png_jmpbuf(png_ptr))) {
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        free(data);
        return NULL;
    }

    struct png_buffer buffer = {};
    buffer.buffer = image;
    buffer.position = 8;

    png_set_read_fn(png_ptr, &buffer, buffer_read);
    png_set_sig_bytes(png_ptr, 8);
    png_read_info(png_ptr, info_ptr);
    build_png_data(png_ptr, info_ptr, data);
    if (!data) {
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        return NULL;
    }
    return data;
    return NULL;
}

void write_png(const char* outfile, struct png_data* data)
{
    FILE* filep = fopen(outfile, "w");
    write_png_fp(filep, data);
    fclose(filep);
}

void write_png_fp(FILE* file, struct png_data* data)
{
    png_structp png_ptr;
    png_infop info_ptr;

    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr) {
        return;
    }

    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        png_destroy_write_struct(&png_ptr, NULL);
        return;
    }

    if (setjmp(png_jmpbuf(png_ptr))) {
        png_destroy_write_struct(&png_ptr, &info_ptr);
        return;
    }

    png_init_io(png_ptr, file);
    png_set_IHDR(png_ptr, info_ptr, data->width,
                 data->height, data->bit_depth,
                 data->color_type, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    png_write_info(png_ptr, info_ptr);

    png_bytep row_pointers[data->height];
    for (uint32_t i = 0; i < data->height; i++) {
        row_pointers[i] = data->data + i * data->width * data->planes;
    }
    png_write_image(png_ptr, row_pointers);
    png_write_end(png_ptr, info_ptr);
    png_destroy_write_struct(&png_ptr, &info_ptr);
}

struct png_buffer* write_png_buffer(struct png_data* png)
{
    png_structp png_ptr;
    png_infop info_ptr;

    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr) {
        return NULL;
    }

    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        png_destroy_write_struct(&png_ptr, NULL);
        return NULL;
    }

    if (setjmp(png_jmpbuf(png_ptr))) {
        png_destroy_write_struct(&png_ptr, &info_ptr);
        return NULL;
    }

    struct png_buffer* buffer = malloc(sizeof(*buffer));
    buffer->buffer = NULL;
    buffer->position = 0;

    png_set_write_fn(png_ptr, buffer, buffer_write, fakeflush);
    png_set_IHDR(png_ptr, info_ptr, png->width,
                 png->height, png->bit_depth,
                 png->color_type, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    png_write_info(png_ptr, info_ptr);

    png_bytep row_pointers[png->height];
    for (uint32_t i = 0; i < png->height; i++) {
        row_pointers[i] = png->data + i * png->width * png->planes;
    }
    png_write_image(png_ptr, row_pointers);
    png_write_end(png_ptr, info_ptr);
    png_destroy_write_struct(&png_ptr, &info_ptr);
    return buffer;
}

static void build_png_data(png_structp png_ptr, png_infop info_ptr, struct png_data* data)
{
    if (setjmp(png_jmpbuf(png_ptr))) {
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        free(data);
        data = NULL;
        return;
    }
    png_get_IHDR(png_ptr, info_ptr, &data->width, &data->height,
                 &data->bit_depth, &data->color_type, NULL, NULL, NULL);
    if (data->bit_depth != 8) {
        // 8-bit images only
        free(data);
        data = NULL;
        return;
    }

    if (data->color_type == PNG_COLOR_TYPE_PALETTE) {
        png_set_palette_to_rgb(png_ptr);
        data->color_type = PNG_COLOR_TYPE_RGB;
    }

    if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) {
        png_set_tRNS_to_alpha(png_ptr);
    }
    png_uint_32 row_bytes;
    png_bytep row_pointers[data->height];

    row_bytes = png_get_rowbytes(png_ptr, info_ptr);
    data->planes = (int)png_get_channels(png_ptr, info_ptr);

    data->data = malloc(row_bytes * data->height);
    if (!data->data) {
        free(data);
        data = NULL;
        return;
    }

    for (png_uint_32 i = 0; i < data->height; i++) {
        row_pointers[i] = data->data + i * row_bytes;
    }

    png_read_image(png_ptr, row_pointers);
    png_read_end(png_ptr, NULL);
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
}
