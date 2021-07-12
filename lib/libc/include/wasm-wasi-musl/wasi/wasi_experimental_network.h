#ifndef __wasi_experimental_network_h
#define __wasi_experimental_network_h

#include <wasi/api.h>
#include <stdbool.h> 

#define IMPORT(name) __attribute__((__import_module__("wasi_experimental_network_unstable"), __import_name__(name)))

typedef uint32_t __wasi_poll_t;

typedef uint32_t __wasi_poll_token_t;

typedef struct __wasi_poll_event_t {
  __wasi_poll_token_t token;
  bool readable;
  bool writable;
} __wasi_poll_event_t;

/**
 * The _domain_ specifies a communication domain; this selects the
 * protocol family which will be used for communication.
 *
 * It uses `i32` which is the equivalent of `int` in C, which is the
 * typed used by `socket(2)` for the `domain` argument.
 */
typedef int32_t __wasi_socket_domain_t;

typedef struct __wasi_socket_address_in_t {
  __wasi_socket_domain_t family;
  uint8_t address[4];
  uint16_t port;
} __wasi_socket_address_in_t;

typedef struct __wasi_socket_address_in6_t {
  __wasi_socket_domain_t family;
  uint16_t sin6_port;
  uint32_t sin6_flowinfo;
  uint8_t sin6_addr[16];
  uint32_t sin6_scope_id;
} __wasi_socket_address_in6_t;

typedef union __wasi_socket_address_t {
  struct __wasi_socket_address_in_t v4;
  struct __wasi_socket_address_in6_t v6;
} __wasi_socket_address_t;

typedef int32_t __wasi_socket_type_t;

typedef int32_t __wasi_socket_protocol_t;

typedef int32_t __wasi_shutdown_t;

// Domains.
#define __WASI_AF_INET 1
#define __WASI_AF_INET6 2
#define __WASI_AF_UNIX 3
// no equivalent defined in `__header_sys_socket.h`:
//#define WASI_AF_PACKET 4
//#define WASI_AF_VSOCK 5

// Protocols.
#define __WASI_DEFAULT_PROTOCOL 0
// no equivalent defined in `__header_sys_socket.h`:
//#define WASI_ICMPv4 1
//#define WASI_ICMPv6 2
//#define WASI_TCP 3
//#define WASI_UDP 4

// Shutdowns.
#define __WASI_SHUT_RD 1
#define __WASI_SHUT_RDWR 3
#define __WASI_SHUT_WR 2

// Types.
#define __WASI_SOCK_DGRAM 2
#define __WASI_SOCK_STREAM 1
// no equivalent defined in `__header_sys_socket.h`:
//#define WASI_SOCK_RAW 4
//#define WASI_SOCK_SEQPACKET 3


IMPORT("poller_add") 
__wasi_errno_t __wasi_experimental_network_poller_add(__wasi_poll_t poll,
                                                      __wasi_fd_t fd,
                                                      struct __wasi_poll_event_t event);

IMPORT("poller_create") 
__wasi_errno_t __wasi_experimental_network_poller_create(__wasi_poll_t *poll_out);

IMPORT("poller_delete") 
__wasi_errno_t __wasi_experimental_network_poller_delete(__wasi_poll_t poll, __wasi_fd_t fd);

IMPORT("poller_modify") 
__wasi_errno_t __wasi_experimental_network_poller_modify(__wasi_poll_t poll,
                                                         __wasi_fd_t fd,
                                                         struct __wasi_poll_event_t event);

IMPORT("poller_wait") 
__wasi_errno_t __wasi_experimental_network_poller_wait(__wasi_poll_t poll,
                                                       struct __wasi_poll_event_t *events,
                                                       uint32_t events_size,
                                                       uint32_t *events_size_out);

IMPORT("socket_accept") 
__wasi_errno_t __wasi_experimental_network_socket_accept(__wasi_fd_t fd,
                                                         union __wasi_socket_address_t *remote_address,
                                                         __wasi_fd_t *remote_fd);

IMPORT("socket_bind") 
__wasi_errno_t __wasi_experimental_network_socket_bind(__wasi_fd_t fd, const union __wasi_socket_address_t *address);

IMPORT("socket_close") 
__wasi_errno_t __wasi_experimental_network_socket_close(__wasi_fd_t fd);

IMPORT("socket_create") 
__wasi_errno_t __wasi_experimental_network_socket_create(__wasi_socket_domain_t domain,
                                                         __wasi_socket_type_t ty,
                                                         __wasi_socket_protocol_t protocol,
                                                         __wasi_fd_t *fd_out);

IMPORT("socket_listen") 
__wasi_errno_t __wasi_experimental_network_socket_listen(__wasi_fd_t fd, uint32_t backlog);

IMPORT("socket_recv") 
__wasi_errno_t __wasi_experimental_network_socket_recv(__wasi_fd_t fd,
                                                       struct __wasi_ciovec_t *iov,
                                                       uint32_t iov_size,
                                                       __wasi_siflags_t iov_flags,
                                                       uint32_t *io_size_out);

IMPORT("socket_send") 
__wasi_errno_t __wasi_experimental_network_socket_send(__wasi_fd_t fd,
                                                       const struct __wasi_ciovec_t *iov,
                                                       uint32_t iov_size,
                                                       __wasi_siflags_t iov_flags,
                                                       uint32_t *io_size_out);

IMPORT("socket_set_nonblocking") 
__wasi_errno_t __wasi_experimental_network_socket_set_nonblocking(__wasi_fd_t fd, bool nonblocking);

IMPORT("socket_shutdown") 
__wasi_errno_t __wasi_experimental_network_socket_shutdown(__wasi_fd_t fd, __wasi_shutdown_t how);

#undef IMPORT

#endif
