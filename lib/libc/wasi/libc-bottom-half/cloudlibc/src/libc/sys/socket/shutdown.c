// Copyright (c) 2015-2016 Nuxi, https://nuxi.nl/
//
// SPDX-License-Identifier: BSD-2-Clause

#include <sys/socket.h>

#include <assert.h>
#include <wasi/api.h>
#include <wasi/wasi_experimental_network.h>
#include <errno.h>

static_assert(SHUT_RD == __WASI_SDFLAGS_RD, "Value mismatch");
static_assert(SHUT_WR == __WASI_SDFLAGS_WR, "Value mismatch");

int shutdown(int fd, int how) {
  __wasi_shutdown_t wasi_how;

  switch (how) {
  case SHUT_RD:
    wasi_how = __WASI_SHUT_RD;
    break;

  case SHUT_WR:
    wasi_how = __WASI_SHUT_WR;
    break;

  case SHUT_RDWR:
    wasi_how = __WASI_SHUT_RDWR;
    break;

  default:
    errno = EINVAL;
    return -1;
  }

  __wasi_errno_t err = __wasi_experimental_network_socket_shutdown(fd, wasi_how);

  if (0 != err) {
    errno = err;
    return -1;
  }

  return err;
}
