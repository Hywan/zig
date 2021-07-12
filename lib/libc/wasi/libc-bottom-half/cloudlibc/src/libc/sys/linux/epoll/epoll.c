#include <errno.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <wasi/wasi_experimental_network.h>

int epoll_create(int size) {
  __wasi_poll_t epfd = 0;
  __wasi_errno_t err = __wasi_experimental_network_poller_create(&epfd);

  if (0 != err) {
    errno = err;
    return -1;
  }

  return epfd;
}


// `flags` will always be overwritten to `EPOLL_CLOEXEC`.
int epoll_create1(int flags) {
  return epoll_create(1);
}

// All events assumed `EPOLLONESHOT` as the `events` flag.
int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event) {
  if (EPOLL_CTL_DEL == op) {
    __wasi_errno_t err = __wasi_experimental_network_poller_delete(epfd, fd);

    if (0 != err) {
      errno = err;
      return -1;
    }

    return 0;
  }

  if (EPOLL_CTL_ADD != op && EPOLL_CTL_MOD != op) {
    errno = EINVAL;
    return -1;
  }

  if (NULL == event) {
    errno = EINVAL;
    return -1;
  }

  struct __wasi_poll_event_t wasi_event = {
    .token = event->data.u32,
    .readable = (event->events & (EPOLLIN | EPOLLRDHUP | EPOLLHUP | EPOLLERR | EPOLLPRI)) != 0,
    .writable = (event->events & (EPOLLOUT | EPOLLHUP | EPOLLERR)) != 0,
  };

  if (EPOLL_CTL_ADD == op) {
    __wasi_errno_t err = __wasi_experimental_network_poller_add(epfd, fd, wasi_event);

    if (0 != err) {
      errno = err;
      return -1;
    }
  } else if (EPOLL_CTL_MOD == op) {
    __wasi_errno_t err = __wasi_experimental_network_poller_modify(epfd, fd, wasi_event);

    if (0 != err) {
      errno = err;
      return -1;
    }
  }

  return 0;
}

int epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout) {
  if (maxevents <= 0) {
    errno = EINVAL;
    return -1;
  }

  __wasi_poll_event_t* received_events = (__wasi_poll_event_t*) malloc(sizeof(__wasi_poll_event_t) * maxevents);
  uint32_t received_events_size = maxevents;

  __wasi_errno_t err = __wasi_experimental_network_poller_wait(epfd, received_events, maxevents, &received_events_size);

  if (0 != err) {
    errno = err;
    return -1;
  }

  if ((int)(received_events_size) > maxevents) {
    return -1;
  }

  for (uint32_t nth = 0; nth < received_events_size; ++nth) {
    __wasi_poll_event_t* received_event = &received_events[nth];

    struct epoll_event* event = &events[nth];

    if (received_event->readable) {
      event->events = EPOLLIN | EPOLLRDHUP | EPOLLHUP | EPOLLERR | EPOLLPRI;
    } else if (received_event->writable) {
      event->events = EPOLLOUT | EPOLLHUP | EPOLLERR;
    } else {
      event->events = 0;
    }

    event->data.u32 = received_event->token;
  }

  return received_events_size;
}

int epoll_pwait(
                int epfd,
                struct epoll_event *events,
                int maxevents,
                int timeout,
                const sigset_t *sigmask
                ) {
  return -1;
}
