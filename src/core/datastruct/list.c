#include "list.h"
#include <stddef.h>

int size = 0;

static struct Node *head = NULL;
static struct Node *tail = NULL;

void add(int data)
{
    struct Node *nPtr, node;
    nPtr = &node;

    if (size == 0) {
        head = nPtr;
        head->next = NULL;
        head->prev = NULL;
        tail = NULL;
    } else {
        tail->prev = nPtr;
        nPtr->next = tail;
        tail->next = NULL;
    }
    size++;
}

void remove(int i)
{

}

void clean()
{

}
