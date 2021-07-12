// Copyright (c) 2015-2017 Nuxi, https://nuxi.nl/
//
// SPDX-License-Identifier: BSD-2-Clause

#include <sys/socket.h>

#include <assert.h>
#include <wasi/api.h>
#include <wasi/wasi_experimental_network.h>
#include <errno.h>
#include <stdint.h>

static_assert(MSG_PEEK == __WASI_RIFLAGS_RECV_PEEK, "Value mismatch");
static_assert(MSG_WAITALL == __WASI_RIFLAGS_RECV_WAITALL, "Value mismatch");

ssize_t recv(int fd, void *restrict buffer, size_t length, int flags) {
  if ((flags & ~(MSG_PEEK | MSG_WAITALL)) != 0) {
    errno = EOPNOTSUPP;
    return -1;
  }

  __wasi_ciovec_t iov = {
    .buf = buffer,
    .buf_len = length
  };
  uint32_t iov_size = 1;
  __wasi_siflags_t iov_flags = flags;
  uint32_t iov_size_out;

  __wasi_errno_t err = __wasi_experimental_network_socket_recv(fd, &iov, iov_size, iov_flags, &iov_size_out);

  if (0 != err) {
    errno = err;
    return -1;
  }

  return iov_size_out;
}
