// SPDX-License-Identifier: GPL 2.0 OR BSD-3-Clause
/*
 *  Copyright 2018-2019 NXP
 */

#ifndef SRC_fifo_H_
#define SRC_fifo_H_

typedef struct
{
	void *pvdata;
} __fentry_t;

typedef struct fifo_tag fifo_t;

fifo_t * fifo_create(const uint32_t depth) __attribute__((cold));
void fifo_destroy(fifo_t *const fifo) __attribute__((cold));
errno_t fifo_put(fifo_t *const fifo, void *const ptr) __attribute__((hot));
void * fifo_get(fifo_t *const fifo) __attribute__((hot));
void * fifo_peek(fifo_t * const fifo, uint32_t num) __attribute__((hot));
errno_t fifo_get_fill_level(fifo_t *const fifo, uint32_t *fill_level) __attribute__((hot));
errno_t fifo_get_free_space(fifo_t *const fifo, uint32_t *free_space) __attribute__((hot));

#endif /* SRC_fifo_H_ */
