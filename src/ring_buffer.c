#include "ring_buffer.h"

#include "utils/coreUtils.h"

struct ring_buffer *create_ring_buffer()
{
    struct ring_buffer *ring_buffer = calloc(1, sizeof(*ring_buffer));

    ring_buffer->names = g_ptr_array_new();
    ring_buffer->index = 0;

    return ring_buffer;
}

void destroy_ring_buffer(struct ring_buffer *ring_buffer)
{
    g_ptr_array_unref(ring_buffer->names);
    free(ring_buffer);
}

const char *ring_buffer_get(struct ring_buffer *ring_buffer)
{
    const char *content = g_ptr_array_index(ring_buffer->names, ring_buffer->index);
    return content;
}

void ring_buffer_rotate(struct ring_buffer *ring_buffer, int i)
{
    ring_buffer->index = relative_index_to_absolute_index(
            ring_buffer->index,
            i,
            ring_buffer->names->len);
}

void ring_buffer_set_names(struct ring_buffer *ring_buffer, GPtrArray *names)
{
    list_clear(ring_buffer->names, NULL);
    wlr_list_cat(ring_buffer->names, names);
}

void ring_buffer_set(struct ring_buffer *dest, struct ring_buffer *src)
{
    ring_buffer_set_names(dest, src->names);
    dest->index = src->index;
}

int relative_index_to_absolute_index(int i, int j, int length)
{
    if (length <= 0)
        return INVALID_POSITION;

    int new_position = (i + j) % length;
    while (new_position < 0)
        new_position += length;

    return new_position;
}



void *get_relative_item(GPtrArray *list, 
        void *(*get_item)(GPtrArray *, int i),
        int (*length_of)(GPtrArray *), int i, int j)
{
    int new_position = relative_index_to_absolute_index(i, j, length_of(list));
    return get_item(list, new_position);
}
