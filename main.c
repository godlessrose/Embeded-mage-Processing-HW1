/* Core/Src/main.c */
#include <stdint.h>
#include <math.h>     // used only to build gamma LUTs once; can be removed if you precompute
#include <string.h>
#include "image.h"    // your grayscale image

// ---------- CONFIG ----------
#define THRESHOLD_LEVEL   128   // Q2-b threshold (change as you like)
#define THR_LOW_VALUE       0
#define THR_HIGH_VALUE    255

// Piecewise linear settings (Q2-d)
// Map [0..r1] -> [0..s1], (r1..r2) -> (s1..s2), [r2..255] -> [s2..255]
#define PWL_r1  64
#define PWL_r2  192
#define PWL_s1  32
#define PWL_s2  224
// -----------------------------

// Mark arrays volatile so the optimizer keeps them in RAM (easy to view in Memory window)
static volatile uint8_t img_src[IMG_N];
static volatile uint8_t img_neg[IMG_N];
static volatile uint8_t img_thr[IMG_N];
static volatile uint8_t img_gamma3[IMG_N];
static volatile uint8_t img_gamma033[IMG_N];
static volatile uint8_t img_pwl[IMG_N];

// Optional: expose symbols for easy watch/memory
volatile uint32_t g_done_flags = 0x0;

// Utility: saturate to [0..255]
static inline uint8_t sat8(int v) {
    if (v < 0)   return 0;
    if (v > 255) return 255;
    return (uint8_t)v;
}

// Q2-a: Negative
static void img_negative(const uint8_t* src, uint8_t* dst, int n) {
    for (int i = 0; i < n; ++i) dst[i] = (uint8_t)(255 - src[i]);
}

// Q2-b: Threshold
static void img_threshold(const uint8_t* src, uint8_t* dst, int n, uint8_t thr, uint8_t lowV, uint8_t highV) {
    for (int i = 0; i < n; ++i) dst[i] = (src[i] >= thr) ? highV : lowV;
}

// Build a gamma LUT for γ = g  (y = 255 * (x/255)^g)
static void build_gamma_lut(float g, uint8_t lut[256]) {
    for (int x = 0; x < 256; ++x) {
        float xn = (float)x / 255.0f;
        float yn = powf(xn, g);
        int y = (int)lroundf(255.0f * yn);
        lut[x] = (uint8_t)((y < 0) ? 0 : (y > 255 ? 255 : y));
    }
}

// Q2-c: Gamma using LUT
static void img_gamma_lut(const uint8_t* src, uint8_t* dst, int n, const uint8_t lut[256]) {
    for (int i = 0; i < n; ++i) dst[i] = lut[src[i]];
}

// Q2-d: Piecewise linear transform (3 segments)
static uint8_t pwl_map(uint8_t x) {
    // Segment 1: [0..r1]
    if (x <= PWL_r1) {
        // y = (s1/r1) * x
        // guard r1==0
        if (PWL_r1 == 0) return 0;
        int y = (int)((x * PWL_s1) / PWL_r1);
        return sat8(y);
    }
    // Segment 2: (r1..r2)
    if (x <= PWL_r2) {
        // y = s1 + (s2 - s1) * (x - r1) / (r2 - r1)
        int num = (int)(PWL_s2 - PWL_s1) * (int)(x - PWL_r1);
        int den = (int)(PWL_r2 - PWL_r1);
        int y = (int)PWL_s1 + (den ? (num / den) : 0);
        return sat8(y);
    }
    // Segment 3: (r2..255]
    // y = s2 + (255 - s2) * (x - r2) / (255 - r2)
    int num = (int)(255 - PWL_s2) * (int)(x - PWL_r2);
    int den = (int)(255 - PWL_r2);
    int y = (int)PWL_s2 + (den ? (num / den) : 0);
    return sat8(y);
}

static void img_pwl_transform(const uint8_t* src, uint8_t* dst, int n) {
    for (int i = 0; i < n; ++i) dst[i] = pwl_map(src[i]);
}
// y = round(255 * (x/255)^3)
static const uint8_t gamma3_lut[256] = {
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   2,
      2,   2,   2,   2,   2,   2,   2,   3,   3,   3,   3,   3,   3,   3,   4,   4,
      4,   4,   4,   5,   5,   5,   5,   6,   6,   6,   6,   6,   7,   7,   7,   8,
      8,   8,   8,   9,   9,   9,  10,  10,  10,  11,  11,  12,  12,  12,  13,  13,
     14,  14,  14,  15,  15,  16,  16,  17,  17,  18,  18,  19,  19,  20,  20,  21,
     22,  22,  23,  23,  24,  25,  25,  26,  27,  27,  28,  29,  29,  30,  31,  32,
     32,  33,  34,  35,  35,  36,  37,  38,  39,  40,  40,  41,  42,  43,  44,  45,
     46,  47,  48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  60,  61,  62,
     63,  64,  65,  67,  68,  69,  70,  72,  73,  74,  76,  77,  78,  80,  81,  82,
     84,  85,  87,  88,  90,  91,  93,  94,  96,  97,  99, 101, 102, 104, 105, 107,
    109, 111, 112, 114, 116, 118, 119, 121, 123, 125, 127, 129, 131, 132, 134, 136,
    138, 140, 142, 144, 147, 149, 151, 153, 155, 157, 159, 162, 164, 166, 168, 171,
    173, 175, 178, 180, 182, 185, 187, 190, 192, 195, 197, 200, 202, 205, 207, 210,
    213, 215, 218, 221, 223, 226, 229, 232, 235, 237, 240, 243, 246, 249, 252, 255
};
// y = round(255 * (x/255)^(1/3))
static const uint8_t gamma033_lut[256] = {
      0,  40,  51,  58,  64,  69,  73,  77,  80,  84,  87,  89,  92,  95,  97,  99,
    101, 103, 105, 107, 109, 111, 113, 114, 116, 118, 119, 121, 122, 124, 125, 126,
    128, 129, 130, 132, 133, 134, 135, 136, 138, 139, 140, 141, 142, 143, 144, 145,
    146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 157, 158, 159, 160,
    161, 162, 163, 163, 164, 165, 166, 167, 167, 168, 169, 170, 170, 171, 172, 173,
    173, 174, 175, 175, 176, 177, 177, 178, 179, 180, 180, 181, 182, 182, 183, 183,
    184, 185, 185, 186, 187, 187, 188, 188, 189, 190, 190, 191, 191, 192, 193, 193,
    194, 194, 195, 196, 196, 197, 197, 198, 198, 199, 199, 200, 201, 201, 202, 202,
    203, 203, 204, 204, 205, 205, 206, 206, 207, 207, 208, 208, 209, 209, 210, 210,
    211, 211, 212, 212, 213, 213, 214, 214, 215, 215, 216, 216, 216, 217, 217, 218,
    218, 219, 219, 220, 220, 221, 221, 221, 222, 222, 223, 223, 224, 224, 224, 225,
    225, 226, 226, 227, 227, 227, 228, 228, 229, 229, 230, 230, 230, 231, 231, 232,
    232, 232, 233, 233, 234, 234, 234, 235, 235, 236, 236, 236, 237, 237, 237, 238,
    238, 239, 239, 239, 240, 240, 241, 241, 241, 242, 242, 242, 243, 243, 243, 244,
    244, 245, 245, 245, 246, 246, 246, 247, 247, 247, 248, 248, 249, 249, 249, 250,
    250, 250, 251, 251, 251, 252, 252, 252, 253, 253, 253, 254, 254, 254, 255, 255
};


int main(void)
{


    // Q1-a: copy “PC image header” into MCU RAM
    // (Your gImageGray[] comes from image.h, which you created on PC.)
    memcpy((void*)img_src, (const void*)gImageGray, IMG_N);

    // Q1-a (visibility): at this point, img_src[] holds raw grayscale values.
    // You can inspect first few entries from the Memory window.

    // Q2: Build gamma LUTs for γ=3 and γ=1/3


    // Q2-a: Negative
    img_negative((const uint8_t*)img_src, (uint8_t*)img_neg, IMG_N);

    // Q2-b: Threshold
    img_threshold((const uint8_t*)img_src, (uint8_t*)img_thr, IMG_N, THRESHOLD_LEVEL, THR_LOW_VALUE, THR_HIGH_VALUE);
    build_gamma_lut(3.0f,  lut_g3);

    build_gamma_lut(1.0f/3.0f, lut_g033);
    // Q2-c: Gamma correction (γ=3 and γ=1/3)
    img_gamma_lut((const uint8_t*)img_src, (uint8_t*)img_gamma3,   IMG_N, gamma3_lut);
    img_gamma_lut((const uint8_t*)img_src, (uint8_t*)img_gamma033, IMG_N, gamma033_lut);

    // Q2-d: Piecewise linear transformation (contrast stretching with two breakpoints)
    img_pwl_transform((const uint8_t*)img_src, (uint8_t*)img_pwl, IMG_N);


    // Set flags so you know processing finished
    g_done_flags = 0xA5A5A5A5;

    // Idle forever
    while (1) {

    }
}
