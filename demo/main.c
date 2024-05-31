// jgabaut @ github.com/jgabaut
// SPDX-License-Identifier: GPL-3.0-only
/*
    Copyright (C) 2022-2024 jgabaut

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
#define RINGABUF_IMPLEMENTATION
#include "../src/ringabuf.h"

typedef struct Example {
    int foo;
} Example;

#define BUFSIZE 50

int main(void) {
    printf("Hello, world!\n");
    printf("Using ringabuf v%s\n", string_ringabuf_version());
    char myBuf[BUFSIZE] = {0};
    RingaBuf rb = rb_new_arr(myBuf, char, BUFSIZE);

    printf("head: %u, tail: %u\n", rb.head, rb.tail);
    bool res = rb_push_bytes(&rb, "Hiiiii", strlen("Hiiiiii"));

    printf("head: %u, tail: %u\n", rb.head, rb.tail);
    printf("res: %s\n", (res ? "true" : "false"));

    char msg[50] = {0};
    size_t res_2 = rb_pop_bytes(&rb, msg, 7);
    printf("res_2: %zu\n", res_2);
    printf("head: %u, tail: %u\n", rb.head, rb.tail);
    printf("msg: %s\n", msg);

    Example ex = (Example) { .foo = 42, };

    bool res_3 = rb_push(&rb, ex);
    printf("res_3: %s\n", (res_3 ? "true" : "false"));
    printf("head: %u, tail: %u\n", rb.head, rb.tail);

    Example ex_2 = {0};

    size_t res_4 = rb_pop(&rb, ex_2);
    printf("res_4: %zu\n", res_4);
    printf("head: %u, tail: %u\n", rb.head, rb.tail);
    printf("ex_2.foo = {%i}\n", ex_2.foo);

    try_rb_push(&rb, ex);
    ex_2 = (Example){0};
    try_rb_pop(&rb, ex_2);
    printf("ex_2.foo = {%i}\n", ex_2.foo);

    return 0;
}
