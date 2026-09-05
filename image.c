#include<stdio.h>
#include<stdlib.h>
#include"image.h"


int load_bmp(const char *filename, IMAGE *img, BMPFILEHEADER *fileHeader, BMPINFOHEADER *infoHeader)
{
    FILE *input;
    int x, y, padding;

    input = fopen(filename, "rb");

    if(input == NULL)
    {
        printf("File could not be opened.\n");
        return 1;
    }

    fread(fileHeader, sizeof(BMPFILEHEADER), 1, input);
    fread(infoHeader, sizeof(BMPINFOHEADER), 1, input);

    if(fileHeader->Type != 0x4D42)
    {
        printf("This is not a BMP file.\n");
        fclose(input);
        return 1;
    }

    printf("This is a BMP file.\n");
    printf("Width: %d\n", infoHeader->Width);
    printf("Height: %d\n", infoHeader->Height);

    img->Width = infoHeader->Width;
    img->Height = infoHeader->Height;
    img->data = malloc(sizeof(RGB) * img->Width * img->Height);

    padding = (4 - (img->Width * 3) % 4) % 4;

    fseek(input, fileHeader->OffBits, SEEK_SET);

    for(y = img->Height - 1; y >= 0; y--)
    {
        for(x = 0; x < img->Width; x++)
        {
            fread(&img->data[y * img->Width + x], sizeof(RGB), 1, input);
        }

        fseek(input, padding, SEEK_CUR);
    }

    fclose(input);
    return 0;
}

int save_bmp(const char *filename, IMAGE *img, BMPFILEHEADER *fileHeader, BMPINFOHEADER *infoHeader)
{
    FILE *output;
    int x, y, padding;
    uint8_t padByte = 0;

    output = fopen(filename, "wb");

    if(output == NULL)
    {
        printf("Output file could not be created.\n");
        return 1;
    }

    infoHeader->Width = img->Width;
    infoHeader->Height = img->Height;

    fwrite(fileHeader, sizeof(BMPFILEHEADER), 1, output);
    fwrite(infoHeader, sizeof(BMPINFOHEADER), 1, output);

    fseek(output, fileHeader->OffBits, SEEK_SET);

    padding = (4 - (img->Width * 3) % 4) % 4;

    for(y = img->Height - 1; y >= 0; y--)
    {
        for(x = 0; x < img->Width; x++)
        {
            fwrite(&img->data[y * img->Width + x], sizeof(RGB), 1, output);
        }

        for(x = 0; x < padding; x++)
        {
            fwrite(&padByte, 1, 1, output);
        }
    }

    fclose(output);
    printf("Image saved to %s\n", filename);
    return 0;
}

int copy_image(IMAGE *src, IMAGE *dst)
{
    int i;

    dst->Width = src->Width;
    dst->Height = src->Height;
    dst->data = malloc(sizeof(RGB) * src->Width * src->Height);

    for(i = 0; i < src->Width * src->Height; i++)
    {
        dst->data[i] = src->data[i];
    }

    return 0;
}

void free_image(IMAGE *img)
{
    if(img->data != NULL)
    {
        free(img->data);
        img->data = NULL;
    }
}

void apply_grayscale(IMAGE *img)
{
    int i;
    int gray;

    for(i = 0; i < img->Width * img->Height; i++)
    {
        gray = 0.114 * img->data[i].rgbBlue
             + 0.587 * img->data[i].rgbGreen
             + 0.299 * img->data[i].rgbRed;

        img->data[i].rgbBlue = gray;
        img->data[i].rgbGreen = gray;
        img->data[i].rgbRed = gray;
    }

    printf("Grayscale filter applied.\n");
}

int apply_brightness(IMAGE *img, int value)
{
    int i;
    int newBlue, newGreen, newRed;

    if(value < -255 || value > 255)
    {
        printf("Invalid brightness value.\n");
        return 1;
    }

    for(i = 0; i < img->Width * img->Height; i++)
    {
        newBlue = img->data[i].rgbBlue + value;
        newGreen = img->data[i].rgbGreen + value;
        newRed = img->data[i].rgbRed + value;

        if(newBlue < 0) newBlue = 0;
        if(newBlue > 255) newBlue = 255;

        if(newGreen < 0) newGreen = 0;
        if(newGreen > 255) newGreen = 255;

        if(newRed < 0) newRed = 0;
        if(newRed > 255) newRed = 255;

        img->data[i].rgbBlue = newBlue;
        img->data[i].rgbGreen = newGreen;
        img->data[i].rgbRed = newRed;
    }

    printf("Brightness adjusted by %d.\n", value);
    return 0;
}

void apply_invert(IMAGE *img)
{
    int i;

    for(i = 0; i < img->Width * img->Height; i++)
    {
        img->data[i].rgbBlue = 255 - img->data[i].rgbBlue;
        img->data[i].rgbGreen = 255 - img->data[i].rgbGreen;
        img->data[i].rgbRed = 255 - img->data[i].rgbRed;
    }

    printf("Image inverted.\n");
}

void apply_hflip(IMAGE *img)
{
    int x, y;
    RGB temp;

    for(y = 0; y < img->Height; y++)
    {
        for(x = 0; x < img->Width / 2; x++)
        {
            temp = img->data[y * img->Width + x];
            img->data[y * img->Width + x] = img->data[y * img->Width + (img->Width - 1 - x)];
            img->data[y * img->Width + (img->Width - 1 - x)] = temp;
        }
    }

    printf("Horizontal flip applied.\n");
}

void apply_vflip(IMAGE *img)
{
    int x, y;
    RGB temp;

    for(y = 0; y < img->Height / 2; y++)
    {
        for(x = 0; x < img->Width; x++)
        {
            temp = img->data[y * img->Width + x];
            img->data[y * img->Width + x] = img->data[(img->Height - 1 - y) * img->Width + x];
            img->data[(img->Height - 1 - y) * img->Width + x] = temp;
        }
    }

    printf("Vertical flip applied.\n");
}

int apply_rotate90(IMAGE *src, IMAGE *dst)
{
    int x, y, newX, newY;

    dst->Width = src->Height;
    dst->Height = src->Width;
    dst->data = malloc(sizeof(RGB) * dst->Width * dst->Height);

    for(y = 0; y < src->Height; y++)
    {
        for(x = 0; x < src->Width; x++)
        {
            newX = src->Height - 1 - y;
            newY = x;

            dst->data[newY * dst->Width + newX] = src->data[y * src->Width + x];
        }
    }

    printf("Image rotated 90 degrees clockwise.\n");
    return 0;
}

int apply_crop(IMAGE *src, IMAGE *dst, int x, int y, int width, int height)
{
    int i, j;

    if(x < 0 || y < 0 || x + width > src->Width || y + height > src->Height)
    {
        printf("Crop region is outside the image boundaries.\n");
        return 1;
    }

    dst->Width = width;
    dst->Height = height;
    dst->data = malloc(sizeof(RGB) * width * height);

    for(j = 0; j < height; j++)
    {
        for(i = 0; i < width; i++)
        {
            dst->data[j * width + i] = src->data[(y + j) * src->Width + (x + i)];
        }
    }

    printf("Image cropped to %dx%d.\n", width, height);
    return 0;
}

int apply_blur(IMAGE *src, IMAGE *dst)
{
    int x, y, dx, dy, nx, ny;
    int sumBlue, sumGreen, sumRed, count;

    dst->Width = src->Width;
    dst->Height = src->Height;
    dst->data = malloc(sizeof(RGB) * dst->Width * dst->Height);

    for(y = 0; y < src->Height; y++)
    {
        for(x = 0; x < src->Width; x++)
        {
            sumBlue = 0;
            sumGreen = 0;
            sumRed = 0;
            count = 0;

            for(dy = -1; dy <= 1; dy++)
            {
                for(dx = -1; dx <= 1; dx++)
                {
                    nx = x + dx;
                    ny = y + dy;

                    if(nx >= 0 && nx < src->Width && ny >= 0 && ny < src->Height)
                    {
                        sumBlue += src->data[ny * src->Width + nx].rgbBlue;
                        sumGreen += src->data[ny * src->Width + nx].rgbGreen;
                        sumRed += src->data[ny * src->Width + nx].rgbRed;
                        count++;
                    }
                }
            }

            dst->data[y * dst->Width + x].rgbBlue = sumBlue / count;
            dst->data[y * dst->Width + x].rgbGreen = sumGreen / count;
            dst->data[y * dst->Width + x].rgbRed = sumRed / count;
        }
    }

    printf("Blur filter applied.\n");
    return 0;
}

int apply_sharpen(IMAGE *src, IMAGE *dst)
{
    int x, y, dx, dy, nx, ny;
    int sumBlue, sumGreen, sumRed, weight;
    int kernel[3][3] = { {0,-1,0}, {-1,5,-1}, {0,-1,0} };

    dst->Width = src->Width;
    dst->Height = src->Height;
    dst->data = malloc(sizeof(RGB) * dst->Width * dst->Height);

    for(y = 0; y < src->Height; y++)
    {
        for(x = 0; x < src->Width; x++)
        {
            sumBlue = 0;
            sumGreen = 0;
            sumRed = 0;

            for(dy = -1; dy <= 1; dy++)
            {
                for(dx = -1; dx <= 1; dx++)
                {
                    nx = x + dx;
                    ny = y + dy;

                    if(nx < 0) nx = 0;
                    if(nx >= src->Width) nx = src->Width - 1;
                    if(ny < 0) ny = 0;
                    if(ny >= src->Height) ny = src->Height - 1;

                    weight = kernel[dy + 1][dx + 1];

                    sumBlue += src->data[ny * src->Width + nx].rgbBlue * weight;
                    sumGreen += src->data[ny * src->Width + nx].rgbGreen * weight;
                    sumRed += src->data[ny * src->Width + nx].rgbRed * weight;
                }
            }

            if(sumBlue < 0) sumBlue = 0;
            if(sumBlue > 255) sumBlue = 255;
            if(sumGreen < 0) sumGreen = 0;
            if(sumGreen > 255) sumGreen = 255;
            if(sumRed < 0) sumRed = 0;
            if(sumRed > 255) sumRed = 255;

            dst->data[y * dst->Width + x].rgbBlue = sumBlue;
            dst->data[y * dst->Width + x].rgbGreen = sumGreen;
            dst->data[y * dst->Width + x].rgbRed = sumRed;
        }
    }

    printf("Sharpen filter applied.\n");
    return 0;
}

IMAGE undoBuffer = { 0, 0, NULL };
int hasUndo = 0;

int save_undo(IMAGE *current)
{
    free_image(&undoBuffer);
    copy_image(current, &undoBuffer);
    hasUndo = 1;

    return 0;
}

int perform_undo(IMAGE *current)
{
    if(hasUndo == 0)
    {
        printf("Nothing to undo.\n");
        return 1;
    }

    free_image(current);

    current->Width = undoBuffer.Width;
    current->Height = undoBuffer.Height;
    current->data = undoBuffer.data;

    undoBuffer.data = NULL;
    hasUndo = 0;

    printf("Undo applied.\n");
    return 0;
}

void clear_undo(void)
{
    free_image(&undoBuffer);
    hasUndo = 0;
}
