#ifndef SERVER_HXX
#define SERVER_HXX

#include <iostream>
#include <memory>
#include <netdb.h>
#include <sys/select.h>
#include <vector>

class HTTPServer {
private:
  int server_socket{};
  int create_socket(const std::unique_ptr<addrinfo *> &bind_address);
  void set_socket_options(int server_socket);
  void bind_socket(int server_socket,
                   const std::unique_ptr<addrinfo *> &bind_address);
  void start_listening(int server_socket);
  void logConnections();

public:
  HTTPServer();
  int getSocket();
  std::string getClientAddr();
  std::unique_ptr<struct client> getClient(std::vector<struct client> &);
  void dropClient();
  fd_set waitForIncommingConnection(std::vector<struct client> &);
  void sendResponse(struct client &);
};

#endif