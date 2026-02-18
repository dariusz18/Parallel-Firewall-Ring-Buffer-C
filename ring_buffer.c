// SPDX-License-Identifier: BSD-3-Clause

#include "ring_buffer.h"
#include <stdlib.h>
#include <string.h>

int ring_buffer_init(so_ring_buffer_t *ring, size_t cap)
{
	/* TODO: implement ring_buffer_init */
	ring->read_pos = 0;
	ring->write_pos = 0;
	ring->stop = 0;
	ring->data = (char *) malloc(cap);
	ring->cap = cap;
	ring->len = 0;
	pthread_mutex_init(&ring->mutex, NULL);
	pthread_cond_init(&ring->are_date, NULL);
	pthread_cond_init(&ring->are_loc, NULL);
	return 1;
}

ssize_t ring_buffer_enqueue(so_ring_buffer_t *ring, void *data, size_t size)
{
	/* TODO: implement ring_buffer_enqueue */
	pthread_mutex_lock(&ring->mutex);
	if (ring->len + size > ring->cap)
		pthread_cond_wait(&ring->are_loc, &ring->mutex);
	char *sursa = (char *) data;
	int pozitie = ring->write_pos;

	for (size_t i = 0; i < size; i++) {
		ring->data[pozitie] = sursa[i];
		pozitie = (pozitie + 1) % ring->cap;
		ring->write_pos = pozitie;
	}
	ring->len = ring->len + size;
	pthread_cond_signal(&ring->are_date);
	pthread_mutex_unlock(&ring->mutex);
	return size;
}

ssize_t ring_buffer_dequeue(so_ring_buffer_t *ring, void *data, size_t size)
{
	/* TODO: implement ring_buffer_dequeue */
	pthread_mutex_lock(&ring->mutex);

	if (ring->stop == 0 && ring->len < size)
		pthread_cond_wait(&ring->are_date, &ring->mutex);
	if (ring->len < size) {
		pthread_mutex_unlock(&ring->mutex);
		return 0;
	}
	char *destinatie = (char *) data;
	int pozitie = ring->read_pos;

	for (size_t i = 0; i < size; i++) {
		destinatie[i] = ring->data[pozitie];
		pozitie = (pozitie + 1) % ring->cap;
		ring->read_pos = pozitie;
	}
	ring->len = ring->len - size;
	pthread_cond_signal(&ring->are_loc);
	pthread_mutex_unlock(&ring->mutex);
	return size;
}

void ring_buffer_destroy(so_ring_buffer_t *ring)
{
	/* TODO: Implement ring_buffer_destroy */
	pthread_mutex_destroy(&ring->mutex);
	free(ring->data);
}

void ring_buffer_stop(so_ring_buffer_t *ring)
{
	/* TODO: Implement ring_buffer_stop */
	pthread_mutex_lock(&ring->mutex);
	ring->stop = 1;
	pthread_mutex_unlock(&ring->mutex);
}
