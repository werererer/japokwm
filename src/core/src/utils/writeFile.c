#include "utils/writeFile.h"
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

static int writeToFile(char *file, char *content)
{
    int filedesc = open(file, O_WRONLY);

    if (filedesc < 0) {
        EBARF("ERROR: file didn't open correctly\n");
        return -1;
    }

    if (write(filedesc, content, sizeof(content)) != sizeof(content)) {
        EBARF("ERROR: failed to write content to file\n");
        return -1;
    }

    return 0;
}

static int writeDoubleToFile(char *file, double d)
{
    int filedesc = open("test.txt", O_WRONLY);
    char content[NUM_DIGITS];
    doubleToString(content, d);
    return writeToFile(file, content);
}

void writeContainerToFile(char *file, struct wlr_fbox *box)
{
    writeDoubleToFile(file, box->x);
    writeToFile(file, " ");
    writeDoubleToFile(file, box->y);
    writeToFile(file, " ");
    writeDoubleToFile(file, box->width);
    writeToFile(file, " ");
    writeDoubleToFile(file, box->height);
    writeToFile(file, "\n");
}

void writeContainerArrayToFile(char *layout, Container **pTexture, 
        size_t width, size_t height)
{
    char base[64] = "layouts";
    char file[64];
    char c[NUM_DIGITS];

    joinPath(base, layout);
    for (int i = 0; i < height; i++) {
        strcpy(file, base);
        intToString(c, i);
        joinPath(file, c);
        mkdir(file, 755);
        for (int j = 0; j < width; j++) {
            writeContainerToFile(file, arr[i][j]);
        }
    }
}
