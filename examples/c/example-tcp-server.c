#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/socket.h>

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
    .sin_port = htons(9010),
  };

  printf("Binding the socket\n");

  int err = bind(server, (struct sockaddr *) &address, sizeof(address));

  if (0 != err) {
    printf("`bind` failed with `%d` (errno = `%d`)\n", err, errno);
    return 2;
  }

  printf("Listening\n");

  err = listen(server, 3);

  if (0 != err) {
    printf("`listen` failed with `%d` (errno = `%d`)\n", err, errno);
    return 3;
  }

  for (;;) {
    printf("Waiting to accept a new connection\n");

    struct sockaddr_in remote_address;
    socklen_t remote_address_len;

    int remote_fd = accept(server, (struct sockaddr *) &remote_address, &remote_address_len);

    if (-1 == remote_fd) {
      printf("`accept` failed with `%d` (errno = `%d`)\n", remote_fd, errno);
      return 4;
    }

    uint8_t buffer[BUFFER_SIZE] = {0};
    ssize_t io_read = recv(remote_fd, &buffer, BUFFER_SIZE, 0);

    if (-1 == io_read) {
      printf("`recv` failed with `%zd` (errno = `%d`)\n", io_read, errno);
      return 5;
    }

    printf("Read: `%.*s`\n", (int) io_read, buffer);

    ssize_t io_written = send(remote_fd, &buffer, io_read, 0);

    if (-1 == io_written) {
      printf("`send` failed with `%zd` (errno = `%d`)\n", io_written, errno);
      return 6;
    }

    err = shutdown(remote_fd, SHUT_RDWR);

    if (0 != err) {
      printf("`shutdown` failed with `%d` (errno = `%d`)\n", err, errno);
      return 7;
    }
  }
}
