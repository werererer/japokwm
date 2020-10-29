#include "utils/writeFile.h"
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

static int writeToFile(int fd, char *content)
{
    if (fd < 0) {
        EBARF("ERROR: file didn't open correctly\n");
        return -1;
    }

    if (write(fd, content, strlen(content)) != strlen(content)) {
        EBARF("ERROR: failed to write content to file\n");
        return -1;
    }
    return 0;
}

static int writeDoubleToFile(int fd, double d)
{
    char content[NUM_CHARS];
    doubleToString(content, d);
    return writeToFile(fd, content);
}

void writeContainerToFile(int fd, Container box)
{
    printf("create container\n");
    writeDoubleToFile(fd, box.x);
    writeToFile(fd, " ");
    writeDoubleToFile(fd, box.y);
    writeToFile(fd, " ");
    writeDoubleToFile(fd, box.width);
    writeToFile(fd, " ");
    writeDoubleToFile(fd, box.height);
    writeToFile(fd, "\n");
}

void writeContainerArrayToFile(int fd, Container box[], size_t length)
{
    for (int i = 0; i < length; i++) {
        writeContainerToFile(fd, box[i]);
    }
}
