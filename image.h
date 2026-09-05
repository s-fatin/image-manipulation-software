#ifndef IMAGE_H
#define IMAGE_H

#include <stdint.h>

#pragma pack(push,1)

typedef struct {
    uint16_t Type;
    uint32_t Size;
    uint16_t Reserved1;
    uint16_t Reserved2;
    uint32_t OffBits;

} BMPFILEHEADER;

typedef struct {
    uint32_t Size;
    int32_t Width;
    int32_t Height;
    uint16_t Planes;
    uint16_t bitperPixel;
    char temp[24];

} BMPINFOHEADER;

#pragma pack(pop)

typedef struct {

    uint8_t rgbBlue;
    uint8_t rgbGreen;
    uint8_t rgbRed;

} RGB;

typedef struct {

    int32_t Width;
    int32_t Height;
    RGB *data;

} IMAGE;

int load_bmp(const char *filename, IMAGE *img, BMPFILEHEADER *fileHeader, BMPINFOHEADER *infoHeader);
int save_bmp(const char *filename, IMAGE *img, BMPFILEHEADER *fileHeader, BMPINFOHEADER *infoHeader);
int copy_image(IMAGE *src, IMAGE *dst);
void free_image(IMAGE *img);

void apply_grayscale(IMAGE *img);
int apply_brightness(IMAGE *img, int value);
void apply_invert(IMAGE *img);
void apply_hflip(IMAGE *img);
void apply_vflip(IMAGE *img);

int apply_rotate90(IMAGE *src, IMAGE *dst);
int apply_crop(IMAGE *src, IMAGE *dst, int x, int y, int width, int height);
int apply_blur(IMAGE *src, IMAGE *dst);
int apply_sharpen(IMAGE *src, IMAGE *dst);

int save_undo(IMAGE *current);
int perform_undo(IMAGE *current);
void clear_undo(void);

#endif
