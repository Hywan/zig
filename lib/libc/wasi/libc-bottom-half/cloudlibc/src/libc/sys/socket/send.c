// Copyright (c) 2015-2017 Nuxi, https://nuxi.nl/
//
// SPDX-License-Identifier: BSD-2-Clause

#include <sys/socket.h>

#include <assert.h>
#include <wasi/api.h>
#include <wasi/wasi_experimental_network.h>
#include <errno.h>

ssize_t send(int fd, const void *buffer, size_t length, int flags) {
  if (flags != 0) {
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

  __wasi_errno_t err = __wasi_experimental_network_socket_send(fd, &iov, iov_size, iov_flags, &iov_size_out);

  if (0 != err) {
    errno = err;
    return -1;
  }

  return iov_size_out;
}
