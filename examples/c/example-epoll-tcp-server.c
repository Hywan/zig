#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#define EVENTS_SIZE 128
#define BUFFER_SIZE 128

int main() {
  printf("Creating the socket\n");

  int server = socket(AF_INET, SOCK_STREAM, 0);

  if (-1 == server) {
    printf("`socket` failed with `%d` (errno = `%d`)\n", server, errno);
    return 1;
  }

  struct sockaddr_in address = {
    .sin_family = AF_INET,
    .sin_addr.s_addr = INADDR_ANY,
    .sin_port = htons(9011),
  };

  printf("Binding the socket\n");

  int err = bind(server, (struct sockaddr *) &address, sizeof(address));

  if (0 != err) {
    printf("`bind` failed with `%d` (errno = `%d`)\n", err, errno);
    return 2;
  }

  printf("Listening\n");

  err = listen(server, 128);

  if (0 != err) {
    printf("`listen` failed with `%d` (errno = `%d`)\n", err, errno);
    return 3;
  }

  printf("Non-blocking mode\n");

  {
    /*
    int old_flags = fcntl(server, F_GETFL);

    if (-1 == old_flags) {
      printf("`fcntl` for `F_GETFL` failed with `%d` (errno = `%d`)\n", old_flags, errno);
      return 4;
    }

    int new_flags = old_flags | O_NONBLOCK;

    printf("old flags = %d, new flags = %d\n", old_flags, new_flags);

    if (old_flags != new_flags) {
      err = fcntl(server, F_SETFL, new_flags);

      if (-1 == err) {
        printf("`fcntl` for `F_SETFL` failed with `%d` (errno = `%d`)\n", err, errno);
        return 5;
      }
    }
    */

    int err = socket_set_nonblocking(server, 1);

    if (0 != err) {
      printf("`socket_set_nonblocking` failed with `%d` (errno = `%d`)\n", err, errno);
      return 4;
    }
  }

  printf("Creating the poller\n");

  int epfd = epoll_create(1);

  if (-1 == epfd) {
    printf("`epoll_create` failed with `%d` (errno = `%d`)\n", epfd, errno);
    return 6;
  }

  printf("Registering the server to the poller\n");

  {
    struct epoll_event event = {
      .events = EPOLLIN | EPOLLONESHOT,
      .data.fd = server,
    };

    err = epoll_ctl(epfd, EPOLL_CTL_ADD, server, &event);

    if (0 != err) {
      printf("`epoll_ctl` with `EPOLL_CTL_ADD` failed with `%d` (errno = `%d`)\n", err, errno);
      return 7;
    }
  }

  printf("Looping\n");

  struct epoll_event events[EVENTS_SIZE];

  while (1) {
    memset(events, 0, sizeof(events)); 

    int number_of_events = 0;

    printf("Waiting for new events\n");

    number_of_events = epoll_wait(epfd, events, EVENTS_SIZE, -1);

    if (-1 == number_of_events) {
      printf("`epoll_wait` failed with `%d` (errno = `%d`)\n", number_of_events, errno);
      return 8;
    }

    printf("Received %d new events\n", number_of_events);

    for (int nth = 0; nth < number_of_events; ++nth) {
      struct epoll_event* event = &events[nth];

      if (event->data.fd == server) {
        printf("Accepting new connections\n");

        while (1) {
          struct sockaddr_in remote_address;
          socklen_t remote_address_len;

          int remote_fd = accept(server, (struct sockaddr *) &remote_address, &remote_address_len);

          if (-1 != remote_fd) {
            // Success.
            printf("Accepted connection (fd = `%d`)\n", remote_fd);
          } else if (EAGAIN == errno) {
            // If we get a `WouldBlock` error, we know our listener
            // has no more incoming connections queued, so we can
            // return to polling and wait for some more.
            break;
          } else {
            // If it was any other kind of error, something went wrong
            // and we terminate with an error.
            printf("`accept` failed with `%d` (errno = `%d`)\n", remote_fd, errno);
            return 9;
          }

          printf("Registering the new connection (only writable events)\n");

          {
            struct epoll_event event = {
              .events = EPOLLOUT | EPOLLONESHOT,
              .data.fd = remote_fd,
            };

            err = epoll_ctl(epfd, EPOLL_CTL_ADD, remote_fd, &event);

            if (0 != err) {
              printf("`epoll_ctl` with `EPOLL_CTL_ADD` failed with `%d` (errno = `%d`)\n", err, errno);
              return 10;
            }
          }
        }

        printf("Re-registering the server\n");

        {
          struct epoll_event event = {
            .events = EPOLLIN | EPOLLOUT | EPOLLONESHOT,
            .data.fd = server,
          };

          err = epoll_ctl(epfd, EPOLL_CTL_MOD, server, &event);

          if (0 != err) {
            printf("`epoll_ctl` with `EPOLL_CTL_MOD` failed with `%d` (errno = `%d`)\n", err, errno);
            return 11;
          }
        }
      } else {
        int client = event->data.fd;

        int close_connection = 0;

        if ((event->events & EPOLLOUT) != 0) {
          printf("Sending “Welcome!” to the client\n");

          ssize_t io_written = send(client, "Welcome!\n", 9, 0);
          
          if (-1 == io_written) {
            printf("`send` failed with `%zd` (errno = `%d`)\n", io_written, errno);
            return 12;
          }

          printf("Re-registering the new connection (only readable events)\n");

          {
            struct epoll_event event = {
              .events = EPOLLIN | EPOLLONESHOT,
              .data.fd = client,
            };

            err = epoll_ctl(epfd, EPOLL_CTL_MOD, client, &event);

            if (0 != err) {
              printf("`epoll_ctl` with `EPOLL_CTL_MOD` failed with `%d` (errno = `%d`)\n", err, errno);
              return 11;
            }
          }

          close_connection = 0;
        } else if ((event->events & EPOLLIN) != 0) {
          printf("Receiving the message from the client\n");

          uint8_t buffer[BUFFER_SIZE] = {0};
          ssize_t io_read = recv(client, &buffer, BUFFER_SIZE, 0);

          if (-1 == io_read) {
            printf("`recv` failed with `%zd` (errno = `%d`)\n", io_read, errno);
            return 12;
          }

          printf("Read: `%.*s`\n", (int) io_read, buffer);

          close_connection = 1;
        } else {
          close_connection = 1;
        }

        if (close_connection == 1) {
          printf("Closing the client %d\n", client);

          socket_close(client);
        }
      }
    }
  }
}
