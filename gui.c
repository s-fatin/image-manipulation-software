#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<iup.h>
#include"image.h"
#include"gui.h"


IMAGE currentImage = { 0, 0, NULL };
BMPFILEHEADER fileHeader;
BMPINFOHEADER infoHeader;
int imageLoaded = 0;
Ihandle *imageWidget = NULL;

#define OP_GRAYSCALE 1
#define OP_INVERT 2
#define OP_HFLIP 3
#define OP_VFLIP 4

#define OP_ROTATE 1
#define OP_BLUR 2
#define OP_SHARPEN 3

void show_message(const char *title, const char *text)
{
    Ihandle *dlg = IupMessageDlg();
    IupSetAttribute(dlg, "DIALOGTYPE", "WARNING");
    IupSetAttribute(dlg, "TITLE", title);
    IupSetAttribute(dlg, "BUTTONS", "OK");
    IupSetAttribute(dlg, "VALUE", text);
    IupPopup(dlg, IUP_CURRENT, IUP_CURRENT);
    IupDestroy(dlg);
}

void refresh_display(void)
{
    int i;
    unsigned char *pixels;
    Ihandle *newImage;
    Ihandle *oldImage;

    if(imageLoaded == 0)
    {
        return;
    }

    pixels = malloc(currentImage.Width * currentImage.Height * 3);

    for(i = 0; i < currentImage.Width * currentImage.Height; i++)
    {
        pixels[i * 3 + 0] = currentImage.data[i].rgbRed;
        pixels[i * 3 + 1] = currentImage.data[i].rgbGreen;
        pixels[i * 3 + 2] = currentImage.data[i].rgbBlue;
    }

    newImage = IupImageRGB(currentImage.Width, currentImage.Height, pixels);
    free(pixels);

    oldImage = (Ihandle *)IupGetAttributeHandle(imageWidget, "IMAGE");

    IupSetAttributeHandle(imageWidget, "IMAGE", newImage);
    IupSetAttribute(imageWidget, "TITLE", NULL);
    IupRefresh(imageWidget);

    if(oldImage != NULL)
    {
        IupDestroy(oldImage);
    }
}

int open_clb(Ihandle *self)
{
    Ihandle *dlg = IupFileDlg();
    IupSetAttribute(dlg, "DIALOGTYPE", "OPEN");
    IupSetAttribute(dlg, "TITLE", "Open BMP Image");
    IupSetAttribute(dlg, "EXTFILTER", "BMP Images (*.bmp)|*.bmp|");

    IupPopup(dlg, IUP_CENTER, IUP_CENTER);

    if(IupGetInt(dlg, "STATUS") != -1)
    {
        char *filename = IupGetAttribute(dlg, "VALUE");

        if(imageLoaded == 1)
        {
            free_image(&currentImage);
            clear_undo();
        }

        if(load_bmp(filename, &currentImage, &fileHeader, &infoHeader) == 0)
        {
            imageLoaded = 1;
            refresh_display();
        }
        else
        {
            imageLoaded = 0;
            show_message("Error", "Could not open that file.");
        }
    }

    IupDestroy(dlg);
    return IUP_DEFAULT;
}

int saveas_clb(Ihandle *self)
{
    Ihandle *dlg;

    if(imageLoaded == 0)
    {
        show_message("No image", "Open an image first.");
        return IUP_DEFAULT;
    }

    dlg = IupFileDlg();
    IupSetAttribute(dlg, "DIALOGTYPE", "SAVE");
    IupSetAttribute(dlg, "TITLE", "Save BMP Image");
    IupSetAttribute(dlg, "EXTFILTER", "BMP Images (*.bmp)|*.bmp|");
    IupSetAttribute(dlg, "EXTDEFAULT", "bmp");

    IupPopup(dlg, IUP_CENTER, IUP_CENTER);

    if(IupGetInt(dlg, "STATUS") != -1)
    {
        char *filename = IupGetAttribute(dlg, "VALUE");
        save_bmp(filename, &currentImage, &fileHeader, &infoHeader);
    }

    IupDestroy(dlg);
    return IUP_DEFAULT;
}

int exit_clb(Ihandle *self)
{
    return IUP_CLOSE;
}

int simple_op_clb(Ihandle *self)
{
    int opcode;

    if(imageLoaded == 0)
    {
        show_message("No image", "Open an image first.");
        return IUP_DEFAULT;
    }

    opcode = IupGetInt(self, "OPCODE");
    save_undo(&currentImage);

    if(opcode == OP_GRAYSCALE)
    {
        apply_grayscale(&currentImage);
    }
    else if(opcode == OP_INVERT)
    {
        apply_invert(&currentImage);
    }
    else if(opcode == OP_HFLIP)
    {
        apply_hflip(&currentImage);
    }
    else if(opcode == OP_VFLIP)
    {
        apply_vflip(&currentImage);
    }

    refresh_display();
    return IUP_DEFAULT;
}

int complex_op_clb(Ihandle *self)
{
    int opcode;
    IMAGE result = { 0, 0, NULL };

    if(imageLoaded == 0)
    {
        show_message("No image", "Open an image first.");
        return IUP_DEFAULT;
    }

    opcode = IupGetInt(self, "OPCODE");

    if(opcode == OP_ROTATE)
    {
        apply_rotate90(&currentImage, &result);
    }
    else if(opcode == OP_BLUR)
    {
        apply_blur(&currentImage, &result);
    }
    else if(opcode == OP_SHARPEN)
    {
        apply_sharpen(&currentImage, &result);
    }

    save_undo(&currentImage);
    free_image(&currentImage);
    currentImage = result;
    refresh_display();

    return IUP_DEFAULT;
}

int undo_clb(Ihandle *self)
{
    if(imageLoaded == 0)
    {
        show_message("No image", "Open an image first.");
        return IUP_DEFAULT;
    }

    perform_undo(&currentImage);
    refresh_display();

    return IUP_DEFAULT;
}

int brightness_clb(Ihandle *self)
{
    Ihandle *input;
    char *text;
    int value;

    if(imageLoaded == 0)
    {
        show_message("No image", "Open an image first.");
        return IUP_DEFAULT;
    }

    input = (Ihandle *)IupGetAttributeHandle(self, "BRIGHTNESS_INPUT");
    text = IupGetAttribute(input, "VALUE");

    if(text == NULL || strlen(text) == 0)
    {
        show_message("Missing value", "Type a brightness value first.");
        return IUP_DEFAULT;
    }

    value = atoi(text);
    save_undo(&currentImage);

    if(apply_brightness(&currentImage, value) != 0)
    {
        perform_undo(&currentImage);
        show_message("Invalid value", "Brightness must be between -255 and 255.");
    }

    refresh_display();
    return IUP_DEFAULT;
}

int crop_clb(Ihandle *self)
{
    Ihandle *xInput, *yInput, *wInput, *hInput;
    int x, y, w, h;
    IMAGE cropped = { 0, 0, NULL };

    if(imageLoaded == 0)
    {
        show_message("No image", "Open an image first.");
        return IUP_DEFAULT;
    }

    xInput = (Ihandle *)IupGetAttributeHandle(self, "CROP_X");
    yInput = (Ihandle *)IupGetAttributeHandle(self, "CROP_Y");
    wInput = (Ihandle *)IupGetAttributeHandle(self, "CROP_W");
    hInput = (Ihandle *)IupGetAttributeHandle(self, "CROP_H");

    x = atoi(IupGetAttribute(xInput, "VALUE"));
    y = atoi(IupGetAttribute(yInput, "VALUE"));
    w = atoi(IupGetAttribute(wInput, "VALUE"));
    h = atoi(IupGetAttribute(hInput, "VALUE"));

    if(apply_crop(&currentImage, &cropped, x, y, w, h) != 0)
    {
        show_message("Invalid crop", "That crop region is outside the image.");
        return IUP_DEFAULT;
    }

    save_undo(&currentImage);
    free_image(&currentImage);
    currentImage = cropped;
    refresh_display();

    return IUP_DEFAULT;
}

Ihandle *make_op_button(const char *label, int opcode, Icallback callback)
{
    Ihandle *btn = IupButton(label, NULL);
    IupSetInt(btn, "OPCODE", opcode);
    IupSetCallback(btn, "ACTION", callback);
    return btn;
}

Ihandle *make_number_input(const char *mask)
{
    Ihandle *input = IupText(NULL);
    IupSetAttribute(input, "MASK", mask);
    IupSetAttribute(input, "VISIBLECOLUMNS", "4");
    return input;
}

void setup_gui(void)
{
    Ihandle *window, *vbox, *hbox;
    Ihandle *btnUndo;
    Ihandle *brightnessInput, *brightnessBtn, *hboxBrightness;
    Ihandle *cropXInput, *cropYInput, *cropWInput, *cropHInput, *cropBtn, *hboxCrop;
    Ihandle *menu, *fileMenu, *fileSubmenu, *itemOpen, *itemSaveAs, *itemExit;

    btnUndo = IupButton("Undo", NULL);
    IupSetCallback(btnUndo, "ACTION", (Icallback)undo_clb);

    hbox = IupHbox(
        make_op_button("Grayscale", OP_GRAYSCALE, (Icallback)simple_op_clb),
        make_op_button("Invert", OP_INVERT, (Icallback)simple_op_clb),
        make_op_button("H-Flip", OP_HFLIP, (Icallback)simple_op_clb),
        make_op_button("V-Flip", OP_VFLIP, (Icallback)simple_op_clb),
        make_op_button("Rotate 90", OP_ROTATE, (Icallback)complex_op_clb),
        make_op_button("Blur", OP_BLUR, (Icallback)complex_op_clb),
        make_op_button("Sharpen", OP_SHARPEN, (Icallback)complex_op_clb),
        btnUndo,
        NULL);
    IupSetAttribute(hbox, "GAP", "4");

    brightnessInput = make_number_input(IUP_MASK_INT);
    brightnessBtn = IupButton("Apply", NULL);
    IupSetAttributeHandle(brightnessBtn, "BRIGHTNESS_INPUT", brightnessInput);
    IupSetCallback(brightnessBtn, "ACTION", (Icallback)brightness_clb);

    hboxBrightness = IupHbox(IupLabel("Brightness (-255 to 255):"), brightnessInput, brightnessBtn, NULL);
    IupSetAttribute(hboxBrightness, "ALIGNMENT", "ACENTER");
    IupSetAttribute(hboxBrightness, "GAP", "4");

    cropXInput = make_number_input(IUP_MASK_UINT);
    cropYInput = make_number_input(IUP_MASK_UINT);
    cropWInput = make_number_input(IUP_MASK_UINT);
    cropHInput = make_number_input(IUP_MASK_UINT);

    cropBtn = IupButton("Crop", NULL);
    IupSetAttributeHandle(cropBtn, "CROP_X", cropXInput);
    IupSetAttributeHandle(cropBtn, "CROP_Y", cropYInput);
    IupSetAttributeHandle(cropBtn, "CROP_W", cropWInput);
    IupSetAttributeHandle(cropBtn, "CROP_H", cropHInput);
    IupSetCallback(cropBtn, "ACTION", (Icallback)crop_clb);

    hboxCrop = IupHbox(
        IupLabel("Crop  X:"), cropXInput,
        IupLabel("Y:"), cropYInput,
        IupLabel("W:"), cropWInput,
        IupLabel("H:"), cropHInput,
        cropBtn, NULL);
    IupSetAttribute(hboxCrop, "ALIGNMENT", "ACENTER");
    IupSetAttribute(hboxCrop, "GAP", "4");

    {
        unsigned char placeholderPixel[3] = { 220, 220, 220 };
        Ihandle *placeholderImage = IupImageRGB(1, 1, placeholderPixel);

        imageWidget = IupLabel(NULL);
        IupSetAttributeHandle(imageWidget, "IMAGE", placeholderImage);
    }

    IupSetAttribute(imageWidget, "EXPAND", "YES");
    IupSetAttribute(imageWidget, "ALIGNMENT", "ACENTER:ACENTER");
    IupSetAttribute(imageWidget, "MINSIZE", "400x300");

    itemOpen = IupItem("Open", NULL);
    IupSetCallback(itemOpen, "ACTION", (Icallback)open_clb);

    itemSaveAs = IupItem("Save As", NULL);
    IupSetCallback(itemSaveAs, "ACTION", (Icallback)saveas_clb);

    itemExit = IupItem("Exit", NULL);
    IupSetCallback(itemExit, "ACTION", (Icallback)exit_clb);

    fileMenu = IupMenu(itemOpen, itemSaveAs, IupSeparator(), itemExit, NULL);
    fileSubmenu = IupSubmenu("File", fileMenu);
    menu = IupMenu(fileSubmenu, NULL);

    vbox = IupVbox(hbox, hboxBrightness, hboxCrop, imageWidget, NULL);
    IupSetAttribute(vbox, "ALIGNMENT", "ACENTER");
    IupSetAttribute(vbox, "GAP", "10");
    IupSetAttribute(vbox, "MARGIN", "10x10");

    window = IupDialog(vbox);
    IupSetAttributeHandle(window, "MENU", menu);
    IupSetAttribute(window, "TITLE", "Image Manipulation Software");
    IupSetAttribute(window, "SIZE", "500x400");

    IupShowXY(window, IUP_CENTER, IUP_CENTER);
    IupMainLoop();
}