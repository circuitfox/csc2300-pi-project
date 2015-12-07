#include <math.h>
#include <stdint.h>

#include "cutil.h"

static const double r_coeff = 0.2126;
static const double g_coeff = 0.7152;
static const double b_coeff = 0.0722;

uint8_t rtvp_rgb_to_l(double r, double g, double b)
{
    return (uint8_t)round(r * r_coeff + g * g_coeff + b * b_coeff);
}
