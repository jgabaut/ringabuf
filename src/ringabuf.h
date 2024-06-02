// jgabaut @ github.com/jgabaut
// SPDX-License-Identifier: GPL-3.0-only
/*
    Copyright (C) 2024 jgabaut

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3 of the License.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#ifndef RINGABUF_H_
#define RINGABUF_H_

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>

#define RINGABUF_MAJOR 0 /**< Represents current major release.*/
#define RINGABUF_MINOR 0 /**< Represents current minor release.*/
#define RINGABUF_PATCH 3 /**< Represents current patch release.*/

/**
 * Defines current API version number from RINGABUF_MAJOR, RINGABUF_MINOR and RINGABUF_PATCH.
 */
static const int RINGABUF_API_VERSION_INT =
    (RINGABUF_MAJOR * 1000000 + RINGABUF_MINOR * 10000 + RINGABUF_PATCH * 100);
/**< Represents current version with numeric format.*/

/**
 * Defines current API version string.
 */
static const char RINGABUF_API_VERSION_STRING[] = "0.0.3"; /**< Represents current version with MAJOR.MINOR.PATCH format.*/

/**
 * Returns current ringabuf version as a string.
 */
const char *string_ringabuf_version(void);

/**
 * Returns current ringabuf version as an integer.
 */
int int_ringabuf_version(void);

typedef struct RingaBuf_s *RingaBuf;

size_t rb_structsize__(void);
size_t rb_structalign__(void);

RingaBuf rb_new_(RingaBuf rb, char* data, size_t elem_size, int count);

#define rb_new_arr(rb, data, type, count) rb_new_((rb), (data), sizeof(type), (count))
#define rb_new(rb, data, type) rb_new_((rb), (data), sizeof(type), 1)

int32_t rb_get_head(RingaBuf rb);
int32_t rb_get_tail(RingaBuf rb);
size_t rb_get_capacity(RingaBuf rb);
size_t rb_get_elem_size(RingaBuf rb);
bool rb_isfull(RingaBuf rb);
char* rb_get_data(RingaBuf rb);
char* rb_getelem_by_offset(RingaBuf rb, int32_t offset, bool* result);
char* rb_getelem_by_index(RingaBuf rb, size_t index, bool* result);
size_t rb_get_newest_idx(RingaBuf rb, bool* result);
char* rb_getelem_newest(RingaBuf rb, bool* result);

bool rb_push_byte(RingaBuf rb, char* data);
size_t rb_push_bytes(RingaBuf rb, char* bytes, size_t count);

#define rb_push(rb, elem) rb_push_bytes((rb), (char*) &(elem), sizeof(elem))
#define try_rb_push(rb, elem) do { \
    size_t rb_inner_size = rb_push((rb), (elem)); \
    if ( rb_inner_size != sizeof(elem)) { \
        fprintf(stderr, "%s():    failed rb_push() call.\n", __func__); \
        exit(EXIT_FAILURE); \
    } \
} while (0);

bool rb_pop_byte(RingaBuf rb, char* data);
size_t rb_pop_bytes(RingaBuf rb, char* bytes, size_t count);
#define rb_pop(rb, elem) rb_pop_bytes((rb), (char*) &(elem), sizeof(elem))
#define try_rb_pop(rb, elem) do { \
    size_t rb_inner_size = rb_pop((rb), (elem)); \
    if ( rb_inner_size != sizeof(elem)) { \
        fprintf(stderr, "%s():    failed rb_pop() call.\n", __func__); \
        exit(EXIT_FAILURE); \
    } \
} while (0);

#endif // RINGABUF_H_

#ifdef RINGABUF_IMPLEMENTATION

struct RingaBuf_s {
    char* data;
    uint32_t head;
    uint32_t tail;
    size_t elem_size;
    size_t capacity;
    bool is_full;
};

size_t rb_structsize__(void)
{
    return sizeof(struct RingaBuf_s);
}

size_t rb_structalign__(void)
{
    return _Alignof(struct RingaBuf_s);
}

RingaBuf rb_new_(RingaBuf rb, char* data, size_t elem_size, int count)
{
    if (rb == NULL) {
        fprintf(stderr, "%s():    Passed RingaBuf was NULL.\n", __func__);
        return rb;
    }
    if (count <= 0) {
        fprintf(stderr,"%s():    invalid count -> {%i}.\n", __func__, count);
        return rb;
    }
    rb->data = data;
    rb->head = 0;
    rb->tail = 0;
    rb->elem_size = elem_size;
    rb->capacity = count * elem_size;
    rb->is_full = false;
    return rb;
}

int32_t rb_get_head(RingaBuf rb)
{
    if (rb == NULL) {
        fprintf(stderr, "%s():    Passed RingaBuf was NULL.\n", __func__);
        return -1;
    }
    return rb->head;
}

int32_t rb_get_tail(RingaBuf rb)
{
    if (rb == NULL) {
        fprintf(stderr, "%s():    Passed RingaBuf was NULL.\n", __func__);
        return -1;
    }
    return rb->tail;
}

size_t rb_get_capacity(RingaBuf rb)
{
    if (rb == NULL) {
        fprintf(stderr, "%s():    Passed RingaBuf was NULL.\n", __func__);
        return 0;
    }
    return rb->capacity;
}

size_t rb_get_elem_size(RingaBuf rb)
{
    if (rb == NULL) {
        fprintf(stderr, "%s():    Passed RingaBuf was NULL.\n", __func__);
        return 0;
    }
    return rb->elem_size;
}

bool rb_isfull(RingaBuf rb)
{
    if (rb == NULL) {
        fprintf(stderr, "%s():    Passed RingaBuf was NULL.\n", __func__);
        return false;
    }
    return rb->is_full;
}

char* rb_get_data(RingaBuf rb)
{
    if (rb == NULL) {
        fprintf(stderr, "%s():    Passed RingaBuf was NULL.\n", __func__);
        return NULL;
    }
    return rb->data;
}

char* rb_getelem_by_offset(RingaBuf rb, int32_t offset, bool* result)
{
    if (rb == NULL) {
        fprintf(stderr, "%s():    Passed RingaBuf was NULL.\n", __func__);
        *result = false;
        return NULL;
    }
    if (offset < 0) {
        fprintf(stderr, "%s():    Passed offset is negative.\n", __func__);
        *result = false;
        return NULL;
    }

    size_t elem_size = rb_get_elem_size(rb);
    if (elem_size < 1) {
#ifndef _WIN32
        fprintf(stderr, "%s():    Invalid elem_size: { %li }\n", __func__, elem_size);
#else
        fprintf(stderr, "%s():    Invalid elem_size: { %lli }\n", __func__, elem_size);
#endif // _WIN32
        *result = false;
        return NULL;
    }

    if (offset % elem_size != 0) {
#ifndef _WIN32
        fprintf(stderr, "%s():    Access at unaligned offset for elem_size { %li }\n", __func__, elem_size);
#else
        fprintf(stderr, "%s():    Access at unaligned offset for elem_size { %lli }\n", __func__, elem_size);
#endif // _WIN32
        *result = false;
    }

    size_t capacity = rb_get_capacity(rb);
    if (offset >= capacity) {
#ifndef _WIN32
        fprintf(stderr, "%s():    Passed offset { %" PRId32 " } is bigger than buffer capacity { %li }\n", __func__, offset, capacity);
#else
        fprintf(stderr, "%s():    Passed offset { %" PRId32 " } is bigger than buffer capacity { %lli }\n", __func__, offset, capacity);
#endif // _WIN32
        *result = false;
        return NULL;
    }
    char* data = rb_get_data(rb);

    if (data == NULL) {
        fprintf(stderr, "%s():    Data was NULL.\n", __func__);
        *result = false;
        return NULL;
    }

    return &(data[offset]);
}

char* rb_getelem_by_index(RingaBuf rb, size_t index, bool* result)
{
    if (rb == NULL) {
        fprintf(stderr, "%s():    Passed RingaBuf was NULL.\n", __func__);
        *result = false;
        return NULL;
    }
    if (index < 0) {
        fprintf(stderr, "%s():    Passed index is negative.\n", __func__);
        *result = false;
        return NULL;
    }

    size_t elem_size = rb_get_elem_size(rb);
    if (elem_size < 1) {
#ifndef _WIN32
        fprintf(stderr, "%s():    Invalid elem_size: { %li }\n", __func__, elem_size);
#else
        fprintf(stderr, "%s():    Invalid elem_size: { %lli }\n", __func__, elem_size);
#endif // _WIN32
        *result = false;
        return NULL;
    }
    size_t capacity = rb_get_capacity(rb);

    if (index > (capacity / elem_size)) {
#ifndef _WIN32
        fprintf(stderr, "%s():    Passed index { %li } is greater than capacity/elem_size { %li }\n", __func__, index, capacity / elem_size);
#else
        fprintf(stderr, "%s():    Passed index { %lli } is greater than capacity/elem_size { %lli }\n", __func__, index, capacity / elem_size);
#endif // _WIN32
        *result = false;
        return NULL;
    }


    if (!rb_isfull(rb)) {
        int32_t head = rb_get_head(rb);
        size_t max_idx = (head / elem_size) -1;
        if (index > max_idx) {
#ifndef _WIN32
            fprintf(stderr, "%s():    Passed index { %li } is greater than max_idx { %li }\n", __func__, index, max_idx);
#else
            fprintf(stderr, "%s():    Passed index { %lli } is greater than max_idx { %lli }\n", __func__, index, max_idx);
#endif // _WIN32
            *result = false;
            return NULL;
        }
    }

    char* data = rb_get_data(rb);

    if (data == NULL) {
        fprintf(stderr, "%s():    Data was NULL.\n", __func__);
        *result = false;
        return NULL;
    }

    return &(data[elem_size * index]);
}

size_t rb_get_newest_idx(RingaBuf rb, bool* result)
{
    if (!rb) {
        fprintf(stderr,"%s():    rb was NULL.\n", __func__);
        *result = false;
        return -1;
    }

    int32_t head = rb_get_head(rb);
    size_t elem_size = rb_get_elem_size(rb);
    size_t capacity = rb_get_capacity(rb);
    if (elem_size < 1) {
#ifndef _WIN32
        fprintf(stderr, "%s():    Invalid elem_size: { %li }\n", __func__, elem_size);
#else
        fprintf(stderr, "%s():    Invalid elem_size: { %lli }\n", __func__, elem_size);
#endif // _WIN32
        *result = false;
        return -1;
    }

    size_t max_idx = (capacity / elem_size) -1;
    size_t newest_idx = -1;

    if (rb_isfull(rb)) {
        newest_idx = ( head == 0 ? (max_idx) : ((head/elem_size) -1));
    } else {
        if (head == 0) {
            fprintf(stderr, "%s():    ring is empty.\n", __func__);
            *result = false;
            return -1;
        }
        newest_idx = ((head/elem_size) -1);
    }

    return newest_idx;
}

char* rb_getelem_newest(RingaBuf rb, bool* result)
{
    bool idx_res = true;
    size_t newest_idx = rb_get_newest_idx(rb, &idx_res);
    if (newest_idx >= 0 && idx_res) {
        return rb_getelem_by_index(rb, newest_idx, result);
    } else {
        if (!idx_res) {
            fprintf(stderr, "%s():    Failed rb_get_newest_idx().\n", __func__);
            *result = false;
            return NULL;
        } else {
#ifndef _WIN32
            fprintf(stderr, "%s():    rb_get_newest_idx() did not fail, but index was negative. {%li}\n", __func__, newest_idx);
#else
            fprintf(stderr, "%s():    rb_get_newest_idx() did not fail, but index was negative. {%lli}\n", __func__, newest_idx);
#endif // _WIN32
            *result = false;
            return NULL;
        }
    }
}

bool rb_push_byte(RingaBuf rb, char* data)
{
    if (!rb) {
        fprintf(stderr,"%s():    rb was NULL.\n", __func__);
        return false;
    }
    if (!data) {
        fprintf(stderr,"%s():    data was NULL.\n", __func__);
        return false;
    }

    rb->data[rb->head] = *data;
    rb->head = (rb->head + 1) % rb->capacity;

    if (rb->is_full) {
        rb->tail = (rb->tail + 1) % rb->capacity;
    }

    rb->is_full = (rb->head == rb->tail);

    return true;
}

size_t rb_push_bytes(RingaBuf rb, char* bytes, size_t count)
{
    if (!rb) {
        fprintf(stderr,"%s():    rb was NULL.\n", __func__);
        return 0;
    }

    size_t elem_size = rb_get_elem_size(rb);
    if (count != elem_size) {
#ifndef _WIN32
        fprintf(stderr,"%s():    Pushing a number of bytes { %li } different than elem_size { %li }\n", __func__,  count, elem_size);
#else
        fprintf(stderr,"%s():    Pushing a number of bytes { %lli } different than elem_size { %lli }\n", __func__, count, elem_size);
#endif // _WIN32
    }

    for (size_t i = 0; i < count; i++) {
        if (!rb_push_byte(rb, &bytes[i])) {
            return i;
        }
    }

    return count;
}

bool rb_pop_byte(RingaBuf rb, char* data)
{
    if (!rb) {
        fprintf(stderr,"%s():    rb was NULL.\n", __func__);
        return false;
    }
    if (!data) {
        fprintf(stderr,"%s():    data was NULL.\n", __func__);
        return false;
    }
    if (rb->head == rb->tail && !rb->is_full) {
        fprintf(stderr,"%s():    Buffer is empty.\n", __func__);
        return false;
    }

    *data = rb->data[rb->tail];
    rb->tail = (rb->tail + 1) % rb->capacity;
    rb->is_full = false;

    return true;
}

size_t rb_pop_bytes(RingaBuf rb, char* bytes, size_t count)
{
    if (!rb) {
        fprintf(stderr,"%s():    rb was NULL.\n", __func__);
        return 0;
    }

    size_t elem_size = rb_get_elem_size(rb);
    if (count != elem_size) {
#ifndef _WIN32
        fprintf(stderr,"%s():    Popping a number of bytes { %li } different than elem_size { %li }\n", __func__,  count, elem_size);
#else
        fprintf(stderr,"%s():    Popping a number of bytes { %lli } different than elem_size { %lli }\n", __func__, count, elem_size);
#endif // _WIN32
    }

    for (size_t i = 0; i < count; i++) {
        if (!rb_pop_byte(rb, bytes + i)) {
            return i;
        }
    }

    return count;
}

const char *string_ringabuf_version(void)
{
    return RINGABUF_API_VERSION_STRING;
}

int int_ringabuf_version(void)
{
    return RINGABUF_API_VERSION_INT;
}

#endif // RINGABUF_IMPLEMENTATION
