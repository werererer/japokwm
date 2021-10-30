#ifndef RING_BUFFER_H
#define RING_BUFFER_H
/*
 * we implement methods here so that we can use a gptrarray like we would use a
 * ring buffer
 * */

#include <glib.h>

struct ring_buffer {
    GPtrArray *names;
    int index;
};

struct ring_buffer *create_ring_buffer();
void destroy_ring_buffer(struct ring_buffer *ring_buffer);

const char *ring_buffer_get(struct ring_buffer *ring_buffer);
void ring_buffer_rotate(struct ring_buffer *ring_buffer, int i);

void ring_buffer_set(struct ring_buffer *dest, struct ring_buffer *src);
void ring_buffer_set_names(struct ring_buffer *ring_buffer, GPtrArray *names);

int relative_index_to_absolute_index(int i, int j, int length);
void *get_relative_item(GPtrArray *list, 
        void *(*get_item)(GPtrArray *, int i),
        int (*length_of)(GPtrArray *), int i, int j);

void prev();
void next();

#endif /* RING_BUFFER_H */
