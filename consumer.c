// SPDX-License-Identifier: BSD-3-Clause

#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>

#include "consumer.h"
#include "ring_buffer.h"
#include "packet.h"
#include "utils.h"

void afisare(struct so_packet_t *pkt, char *output)
{
	if (process_packet(pkt))
		sprintf(output, "PASS %016lx %lu\n", packet_hash(pkt), pkt->hdr.timestamp);
	else
		sprintf(output, "DROP %016lx %lu\n", packet_hash(pkt), pkt->hdr.timestamp);
}

void consumer_thread(so_consumer_ctx_t *ctx)
{
	/* TODO: implement consumer thread */
	char buffer[PKT_SZ];
	char out[PKT_SZ];
	int fd = open(ctx->out, O_WRONLY | O_CREAT | O_APPEND, 0755);

	while (ring_buffer_dequeue(ctx->producer_rb, buffer, PKT_SZ) > 0) {
		struct so_packet_t *pkt = (struct so_packet_t *)buffer;

		afisare(pkt, out);
		write(fd, out, strlen(out));
	}
	close(fd);
	free(ctx);
}

int create_consumers(pthread_t *tids,
					 int num_consumers,
					 struct so_ring_buffer_t *rb,
					 const char *out_filename)
{
	for (int i = 0; i < num_consumers; i++) {
		so_consumer_ctx_t *ctx = malloc(sizeof(*ctx));

		ctx->producer_rb = rb;

		ctx->out = out_filename;
		pthread_create(&tids[i], NULL, (void *)consumer_thread, ctx);
	}
	return num_consumers;
}
