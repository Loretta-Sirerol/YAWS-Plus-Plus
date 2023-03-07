#ifndef CLIENT_HXX
#define CLIENT_HXX

#include <sys/socket.h>

struct client {
  socklen_t address_length;
  struct sockaddr_storage address;
  char address_buffer[128];
  int socket;
  char request[4096];
  int received;
};

#endif
