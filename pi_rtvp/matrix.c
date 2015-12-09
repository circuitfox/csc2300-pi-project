#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include "matrix.h"

static uint8_t* corner_matrix_tl(uint8_t* data, int y, int x, int width, int size);
static uint8_t* corner_matrix_bl(uint8_t* data, int y, int x, int width, int size);
static uint8_t* corner_matrix_tr(uint8_t* data, int y, int x, int width, int size);
static uint8_t* corner_matrix_br(uint8_t* data, int y, int x, int width, int size);
static void corner_row_left(uint8_t* data, uint8_t* matrix,
                            int lidx, int hidx,
                            int median, int* index);
static void corner_row_right(uint8_t* data, uint8_t* matrix,
                             int lidx, int hidx,
                             int median, int* index);

static uint8_t* edge_matrix_top(uint8_t* data, int y, int x, int width, int size);
static uint8_t* edge_matrix_bottom(uint8_t* data, int y, int x, int width, int size);
static uint8_t* edge_matrix_right(uint8_t* data, int y, int x, int width, int size);
static uint8_t* edge_matrix_left(uint8_t* data, int y, int x, int width, int size);

static int all_equal(uint8_t* data, int len)
{
    uint8_t val = data[0];
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
    res /= len;
    return (uint8_t)round(res);
}

uint8_t
convolve_id(uint8_t* image_chunk, int size)
{
    return image_chunk[size*size/2];
}

uint8_t
convolve_gaussian(uint8_t* image_chunk, uint16_t* kernel, int size)
{
    double res = 0;
    int ksum = 0;
    int len = size * size;
    if (all_equal(image_chunk, len)) {
        return image_chunk[0];
    }
    for (int i = 0; i < len; ++i) {
        uint16_t k = kernel[i];
        res += image_chunk[i] * k;
        ksum += k;
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

uint8_t* convolve_image(uint8_t* data, uint8_t* kernel,
                        int height, int width, int size)
{
    uint8_t* processed = malloc(width * height);
    int index = 0;
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            uint8_t* chunk = get_matrix_at(data, i, j, height, width, size);
            processed[index] = convolve(chunk, kernel, size);
            index++;
            free(chunk);
        }
    }
    return processed;
}

uint8_t* convolve_image_id(uint8_t* data, int height, int width, int size)
{
    uint8_t* processed = malloc(width * height);
    int index = 0;
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            uint8_t* chunk = get_matrix_at(data, i, j, height, width, size);
            processed[index] = convolve_id(chunk, size);
            index++;
            free(chunk);
        }
    }
    return processed;
}

uint8_t* convolve_image_gaussian(uint8_t* data, uint16_t* kernel,
                                 int height, int width, int size)
{
    uint8_t* processed = malloc(width * height);
    int index = 0;
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            uint8_t* chunk = get_matrix_at(data, i, j, height, width, size);
            processed[index] = convolve_gaussian(chunk, kernel, size);
            index++;
            free(chunk);
        }
    }
    return processed;
}

uint8_t* convolve_image_sobel(uint8_t* data, int8_t* kernel_x, int8_t* kernel_y,
                              int height, int width, int size)
{
    uint8_t* processed = malloc(width * height);
    int index = 0;
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            uint8_t* chunk = get_matrix_at(data, i, j, height, width, size);
            processed[index] = convolve_sobel(chunk, kernel_x, kernel_y, size);
            index++;
            free(chunk);
        }
    }
    return processed;
}

int is_edge(int y, int x, int height, int width, int median)
{
    enum edge_type type;
    if (y - median < 0) {
        type = EDGE_TOP;
    } else if (y + median > height - 1) {
        type = EDGE_BOTTOM;
    } else if (x - median < 0) {
        type = EDGE_LEFT;
    } else if ( x + median > width - 1) {
        type = EDGE_RIGHT;
    } else {
        type = EDGE_NONE;
    }
    return type;
}

int is_corner(int y, int x, int height, int width, int median)
{
    enum corner_type type;
    if (x - median < 0 && y - median < 0) {
        type = CORNER_TOPLEFT;
    } else if (x - median < 0 && y + median > height - 1) {
        type = CORNER_BOTTOMLEFT;
    } else if (x + median > width - 1 && y - median < 0) {
        type = CORNER_TOPRIGHT;
    } else if (x + median > width - 1 && y + median > height - 1) {
        type = CORNER_BOTTOMRIGHT;
    } else {
        type = CORNER_NONE;
    }
    return type;
}

uint8_t* get_matrix_at(uint8_t* data, int y, int x, int height, int width, int size)
{
    int median = size / 2;
    uint8_t* matrix;
    int type;
    if ((type = is_corner(y, x, height, width, median))) {
        matrix = corner_matrix(data, y, x, width, size, type);
    } else if ((type = is_edge(y, x, height, width, median))) {
        matrix = edge_matrix(data, y, x, width, size, type);
    } else {
        matrix = center_matrix(data, y, x, width, size);
    }
    return matrix;
}

uint8_t* center_matrix(uint8_t* data, int y, int x, int width, int size)
{
    int median = size / 2;
    uint8_t* mat = malloc(size * size);
    if (!mat) {
        return NULL;
    }
    int yl = (y - median) * width;
    int yh = (y + median) * width;
    int xl = x - median;
    int xh = x + median;
    int index = 0;
    for (int row = yl; row <= yh; row += width) {
        for (int col = xl; col <= xh; ++col) {
            mat[index] = data[row + col];
            index++;
        }
    }
    return mat;
}

uint8_t* corner_matrix(uint8_t* data, int y, int x,
                       int width, int size, enum corner_type type)
{
    uint8_t* matrix;
    switch (type) {
    case CORNER_TOPLEFT:
        matrix = corner_matrix_tl(data, y, x, width, size);
        break;
    case CORNER_BOTTOMLEFT:
        matrix = corner_matrix_bl(data, y, x, width, size);
        break;
    case CORNER_TOPRIGHT:
        matrix = corner_matrix_tr(data, y, x, width, size);
        break;
    case CORNER_BOTTOMRIGHT:
        matrix = corner_matrix_br(data, y, x, width, size);
        break;
    default:
        matrix = NULL;
    }
    return matrix;
}

uint8_t* edge_matrix(uint8_t* data, int y, int x,
                     int width, int size, enum edge_type type)
{
    uint8_t* matrix;
    switch (type) {
    case EDGE_TOP:
        matrix = edge_matrix_top(data, y, x, width, size);
        break;
    case EDGE_BOTTOM:
        matrix = edge_matrix_bottom(data, y, x, width, size);
        break;
    case EDGE_LEFT:
        matrix = edge_matrix_left(data, y, x, width, size);
        break;
    case EDGE_RIGHT:
        matrix = edge_matrix_right(data, y, x, width, size);
        break;
    default:
        matrix = NULL;
        break;
    }
    return matrix;
};

static uint8_t* corner_matrix_tl(uint8_t* data, int y, int x, int width, int size)
{
    int median = size / 2;
    int index = 0;
    int yl = y * width;
    int yh = (y + median) * width;
    int xh = x + median;
    uint8_t* matrix = malloc(size * size);
    if (!matrix) {
        return NULL;
    }
    while (index < median*size) {
        corner_row_left(data, matrix, yl+x, yl+xh, median, &index);
    }
    for (int row = yl; row <= yh; row += width) {
        corner_row_left(data, matrix, row+x, row+xh, median, &index);
    }
    return matrix;
}

static uint8_t* corner_matrix_bl(uint8_t* data, int y, int x, int width, int size)
{
    int median = size / 2;
    int index = 0;
    int yl = (y - median) * width;
    int yh = y * width;
    int xh = x + median;
    uint8_t* matrix = malloc(size * size);
    if (!matrix) {
        return NULL;
    }
    for (int row = yl; row <= yh; row += width) {
        corner_row_left(data, matrix, row+x, row+xh, median, &index);
    }
    while (index < size*size) {
        corner_row_left(data, matrix, yh+x, yh+xh, median, &index);
    }
    return matrix;
}

static uint8_t* corner_matrix_tr(uint8_t* data, int y, int x, int width, int size)
{
    int median = size / 2;
    int index = 0;
    int yl = y * width;
    int yh = (y + median) * width;
    int xl = x - median;
    uint8_t* matrix = malloc(size * size);
    if (!matrix) {
        return NULL;
    }
    while (index < median*size) {
        corner_row_right(data, matrix, yl+xl, yl+x, median, &index);
    }
    for (int row = yl; row <= yh; row += width) {
        corner_row_right(data, matrix, row+xl, row+x, median, &index);
    }
    return matrix;
}

static uint8_t* corner_matrix_br(uint8_t* data, int y, int x, int width, int size)
{
    int median = size / 2;
    int index = 0;
    int yl = (y - median) * width;
    int yh = y * width;
    int xl = x - median;
    uint8_t* matrix = malloc(size * size);
    if (!matrix) {
        return NULL;
    }
    for (int row = yl; row <= yh; row += width) {
        corner_row_right(data, matrix, row+xl, row+x, median, &index);
    }
    while (index < size*size) {
        corner_row_right(data, matrix, yh+xl, yh+x, median, &index);
    }
    return matrix;
}

static void corner_row_left(uint8_t* data, uint8_t* matrix, 
                            int lidx, int hidx,
                            int median, int* index)
{
    for (int i = 0; i < median; ++i) {
        matrix[*index] = data[lidx];
        (*index)++;
    }
    for (int col = lidx; col <= hidx; ++col) {
        matrix[*index] = data[col];
        (*index)++;
    }
}
static void corner_row_right(uint8_t* data, uint8_t* matrix,
                             int lidx, int hidx,
                             int median, int* index)
{
    for (int col = lidx; col <= hidx; ++col) {
        matrix[*index] = data[col];
        (*index)++;
    }
    for (int i = 0; i < median; ++i) {
        matrix[*index] = data[hidx];
        (*index)++;
    }

}

static uint8_t* edge_matrix_top(uint8_t* data, int y, int x, int width, int size)
{

    int median = size / 2;
    int index = 0;
    int yl = y * width;
    int yh = (y + median) * width;
    int xl = x - median;
    int xh = x + median;
    uint8_t* matrix = malloc(size * size);
    if (!matrix) {
        return NULL;
    }
    while (index < median*size) {
        for (int col = xl; col <= xh; ++col) {
            matrix[index] = data[yl+col];
            index++;
        }
    }
    for (int row = yl; row <= yh; row += width) {
        for (int col = xl; col <= xh; ++col) {
            matrix[index] = data[row+col];
            index++;
        }
    }
    return matrix;
}

static uint8_t* edge_matrix_bottom(uint8_t* data, int y, int x, int width, int size)
{
    int median = size / 2;
    int index = 0;
    int yl = (y - median) * width;
    int yh = y * width;
    int xl = x - median;
    int xh = x + median;
    uint8_t* matrix = malloc(size * size);
    if (!matrix) {
        return NULL;
    }
    for (int row = yl; row <= yh; row += width) {
        for (int col = xl; col <= xh; ++col) {
            matrix[index] = data[row+col];
            index++;
        }
    }
    while (index < median*size) {
        for (int col = xl; col <= xh; ++col) {
            matrix[index] = data[yh+col];
            index++;
        }
    }
    return matrix;
}

static uint8_t* edge_matrix_left(uint8_t* data, int y, int x, int width, int size)
{
    int median = size / 2;
    int index = 0;
    int yl = (y - median) * width;
    int yh = (y + median) * width;
    int xl = x;
    int xh = x + median;
    uint8_t* matrix = malloc(size * size);
    if (!matrix) {
        return NULL;
    }
    for (int row = yl; row <= yh; row += width) {
        for (int i = 0; i < median; ++i) {
            matrix[index] = data[row+xl];
            index++;
        }
        for (int col = xl; col <= xh; ++col) {
            matrix[index] = data[row+col];
            index++;
        }
    }
    return matrix;
}

static uint8_t* edge_matrix_right(uint8_t* data, int y, int x, int width, int size)
{
    int median = size / 2;
    int index = 0;
    int yl = (y - median) * width;
    int yh = (y + median) * width;
    int xl = x - median;
    int xh = x;
    uint8_t* matrix = malloc(size * size);
    if (!matrix) {
        return NULL;
    }
    for (int row = yl; row <= yh; row += width) {
        for (int col = xl; col <= xh; ++col) {
            matrix[index] = data[row+col];
            index++;
        }
        for (int i = 0; i < median; ++i) {
            matrix[index] = data[row+xh];
            index++;
        }
    }
    return matrix;
}
