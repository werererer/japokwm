#ifndef LIST_H
#define LIST_H
struct Node {
    int data;
    struct Node *next;
    struct Node *prev;
};

extern int size;

void add(int data);
void remove(int i);
void clean();
#endif /* LIST_H */
