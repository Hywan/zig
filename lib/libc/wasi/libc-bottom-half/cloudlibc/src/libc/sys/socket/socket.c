#include <errno.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <wasi/wasi_experimental_network.h>

static int to_wasi_domain(int domain, __wasi_socket_domain_t *wasi_domain) {
  switch (domain) {
  case AF_INET:
    *wasi_domain = __WASI_AF_INET;
    break;

  case AF_INET6:
    *wasi_domain = __WASI_AF_INET6;
    break;

  case AF_UNIX:
    *wasi_domain = __WASI_AF_UNIX;

  default:
    return -1;
  }

  return 0;
}

static int to_wasi_type(int type, __wasi_socket_type_t *wasi_type) {
  switch (type) {
  case SOCK_DGRAM:
    *wasi_type = __WASI_SOCK_DGRAM;
    break;

  case SOCK_STREAM:
    *wasi_type = __WASI_SOCK_STREAM;
    break;

  default:
    return -1;
  }

  return 0;
}

static int to_wasi_protocol(int protocol, __wasi_socket_protocol_t *wasi_protocol) {
  switch (protocol) {
  case 0:
    *wasi_protocol = __WASI_DEFAULT_PROTOCOL;
    break;

  default:
    return -1;
  }

  return 0;
}

int socket(int domain, int type, int protocol) {
  __wasi_socket_domain_t wasi_domain;
  __wasi_socket_type_t wasi_type;
  __wasi_socket_protocol_t wasi_protocol;
  __wasi_fd_t fd;
  __wasi_errno_t err;

  if (0 != to_wasi_domain(domain, &wasi_domain)) {
    errno = EAFNOSUPPORT;
    return -1;
  }

  if (0 != to_wasi_type(type, &wasi_type)) {
    errno = EACCES;
    return -1;
  }

  if (0 != to_wasi_protocol(protocol, &wasi_protocol)) {
    errno = EPROTONOSUPPORT;
    return -1;
  }

  err = __wasi_experimental_network_socket_create(wasi_domain, wasi_type, wasi_protocol, &fd);
    
  if (0 != err) {
    errno = err;
    return -1;
  }

  return fd;
}

int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
  return -1;
}

int bind(int fd, const struct sockaddr *addr, socklen_t addrlen) {
  union __wasi_socket_address_t wasi_address;
  __wasi_errno_t err;

  // IPv4.
  if (addrlen == sizeof(struct sockaddr_in)) {
    const struct sockaddr_in *ipv4 = (const struct sockaddr_in *) addr;
    __wasi_socket_domain_t wasi_domain;

    if (-1 == to_wasi_domain(ipv4->sin_family, &wasi_domain)) {
      errno = EAFNOSUPPORT;
      return -1;
    }

    union address_t {
      in_addr_t s_addr;
      uint8_t address[4];
    };

    union address_t address;
    address.s_addr = ipv4->sin_addr.s_addr;

    wasi_address.v4.family = wasi_domain;
    wasi_address.v4.address[0] = address.address[0];
    wasi_address.v4.address[1] = address.address[1];
    wasi_address.v4.address[2] = address.address[2];
    wasi_address.v4.address[3] = address.address[3];
    wasi_address.v4.port = ipv4->sin_port;
  } else {
    // TODO: not fully implemented for the moment
    return -1;
  }

  err = __wasi_experimental_network_socket_bind(fd, &wasi_address);

  if (0 != err) {
    errno = err;
    return -1;
  }

  return 0;
}

int listen(int fd, int backlog) {
  __wasi_errno_t err = __wasi_experimental_network_socket_listen(fd, backlog);

  if (0 != err) {
    errno = err;
    return -1;
  }

  return 0;
}

int accept(int fd, struct sockaddr *addr, socklen_t *addrlen) {
  union __wasi_socket_address_t remote_address;
  __wasi_fd_t remote_fd;

  __wasi_errno_t err = __wasi_experimental_network_socket_accept(fd, &remote_address, &remote_fd);

  if (0 != err) {
    errno = err;
    return -1;
  }

  if (NULL != addr && NULL != addrlen) {
    // IPv4
    if (remote_address.v4.family == __WASI_AF_INET) {
      struct sockaddr_in *ipv4 = (struct sockaddr_in *) addr;

      union address_t {
        in_addr_t s_addr;
        uint8_t address[4];
      };

      union address_t address;
      address.address[0] = remote_address.v4.address[0];
      address.address[1] = remote_address.v4.address[1];
      address.address[2] = remote_address.v4.address[2];
      address.address[3] = remote_address.v4.address[3];

      ipv4->sin_family = AF_INET;
      ipv4->sin_port = remote_address.v4.port;
      ipv4->sin_addr.s_addr = address.s_addr;

      *addrlen = sizeof(struct sockaddr_in);
    } else {
      // TODO: not fully implemented for the moment
      return -1;
    }
  }

  return remote_fd;
}

int accept4(int sockfd, struct sockaddr *addr, socklen_t *addrlen, int flags) {
  return accept(sockfd, addr, addrlen);
}

int setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen) {
  return 0;
}

int socket_set_nonblocking(int sockfd, int nonblocking) {
  if (0 != __wasi_experimental_network_socket_set_nonblocking(sockfd, 1 == nonblocking)) {
    return -1;
  }

  return 0;
}

int socket_close(int sockfd) {
  if (0 != __wasi_experimental_network_socket_close(sockfd)) {
    return -1;
  }

  return 0;
}
