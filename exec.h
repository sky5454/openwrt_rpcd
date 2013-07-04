/*
 * luci-rpcd - LuCI UBUS RPC server
 *
 *   Copyright (C) 2013 Jo-Philipp Wich <jow@openwrt.org>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef __RPC_EXEC_H
#define __RPC_EXEC_H

#include <libubus.h>
#include <libubox/blobmsg.h>
#include <libubox/ustream.h>

#define RPC_EXEC_MAX_SIZE		(4096 * 64)
#define RPC_EXEC_MAX_RUNTIME	(3 * 1000)

#define ustream_for_each_read_buffer(stream, ptr, len) \
	for (ptr = ustream_get_read_buf(stream, &len);     \
	     ptr != NULL && len > 0;                       \
	     ustream_consume(stream, len), ptr = ustream_get_read_buf(stream, &len))

#define ustream_declare(us, fd, name)                     \
	us.stream.string_data   = true;                       \
	us.stream.r.buffer_len  = 4096;                       \
	us.stream.r.max_buffers = RPC_EXEC_MAX_SIZE / 4096;   \
	us.stream.notify_read   = rpc_exec_##name##_read_cb;  \
	us.stream.notify_state  = rpc_exec_##name##_state_cb; \
	ustream_fd_init(&us, fd);

typedef int (*rpc_exec_read_cb_t)(struct blob_buf *, char *, int, void *);
typedef void (*rpc_exec_done_cb_t)(struct blob_buf *, int, void *);

struct rpc_exec_context {
	struct ubus_context *context;
	struct ubus_request_data request;
	struct uloop_timeout timeout;
	struct uloop_process process;
	struct ustream_fd opipe;
	struct ustream_fd epipe;
	int outlen;
	char *out;
	int errlen;
	char *err;
	int stat;
	void *priv;
	bool blob_array;
	void *blob_cookie;
	struct blob_buf blob;
	rpc_exec_read_cb_t stdout_cb;
	rpc_exec_read_cb_t stderr_cb;
	rpc_exec_done_cb_t finish_cb;
};

int rpc_exec(const char **args, rpc_exec_read_cb_t out, rpc_exec_read_cb_t err,
             rpc_exec_done_cb_t end, void *priv, struct ubus_context *ctx,
             struct ubus_request_data *req);

#endif