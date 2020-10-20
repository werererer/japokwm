#include "utils/writeFile.h"
#include "utils/stringUtils.h"
#include <stdio.h>
#include <string.h>

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

static int writeLnToFile(char *file, char *content)
{
    strcat(content, "\n");
    return writeToFile(file, content);
}

static int writeDoubleToFile(char *file, double d)
{
    int filedesc = open("test.txt", O_WRONLY);
    char content[NUM_DIGITS];
    doubleToString(content, d);
    return writeToFile(file, content);
}

int writeContainerToFile(char *file, struct wlr_fbox *box)
{
    char *container = "Container";
    writeToFile(file, container);
    writeToFile(file, "(");
    writeDoubleToFile(file, box->x);
    writeToFile(file, ",");
    writeDoubleToFile(file, box->y);
    writeToFile(file, ",");
    writeDoubleToFile(file, box->width);
    writeToFile(file, ",");
    writeDoubleToFile(file, box->height);
    writeToFile(file, ")");
}

static int writeIndentToFile(char *file, int indentLevel)
{
    char *indent = "    ";
    repeatString(indent, 5);
    return writeToFile(file, indent);
}

void writeArrayToFile(char *file, struct wlr_fbox ***arr, 
        size_t width, size_t height)
{
    writeLnToFile(file, layoutBegin);
    for (int i = 0; i < height; i++) {
        writeIndentToFile(file, 1);
        writeLnToFile(file, "[");
        for (int j = 0; j < width; j++) {
            writeIndentToFile(file, 2);
            writeLnToFile(file, "[");
            writeIndentToFile(file, 3);
            writeContainerToFile(file, arr[i][j]);
            writeIndentToFile(file, 2);
            writeLnToFile(file, "]");
        }
        writeIndentToFile(file, 1);
        writeLnToFile(file, "]");
    }
}
