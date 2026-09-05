#include<stdio.h>
#include<stdlib.h>
#include<iup.h>
#include"gui.h"

int main(int argc, char **argv)
{
    IupOpen(&argc, &argv);
    setup_gui();
    IupClose();

    return 0;
}
