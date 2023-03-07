#include "headers/Server.hxx"
#include "headers/Client.hxx"
#include <memory>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

HTTPServer::HTTPServer() {
  std::cout << "Configuring local address..." << std::endl;

  addrinfo hints{};
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  std::unique_ptr<addrinfo *> bind_address = std::make_unique<addrinfo *>();

  int status = getaddrinfo(0, "8080", &hints, bind_address.get());
  if (status != 0) {
    throw std::runtime_error{"getaddrinfo() failed with error number " +
                             std::to_string(status)};
  }

  std::cout << "Creating socket..." << std::endl;
  server_socket = create_socket(bind_address);

  std::cout << "Setting socket options" << std::endl;
  set_socket_options(server_socket);

  std::cout << "Binding socket to local address..." << std::endl;
  bind_socket(server_socket, bind_address);

  std::cout << "Listening..." << std::endl;
  start_listening(server_socket);

  std::cout << "Server is listening on port 8080" << std::endl;
}

int HTTPServer::create_socket(const std::unique_ptr<addrinfo *> &bind_address) {
  int server_socket =
      socket((*bind_address)->ai_family, (*bind_address)->ai_socktype,
             (*bind_address)->ai_protocol);
  if (server_socket < 0) {
    throw std::runtime_error{"socket() failed with error number " +
                             std::to_string(errno)};
  }

  return server_socket;
}

void HTTPServer::set_socket_options(int server_socket) {
  int opt = 1;
  if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) <
      0) {
    throw std::runtime_error{"setsockopt() failed with error number " +
                             std::to_string(errno)};
  }
}

void HTTPServer::bind_socket(int server_socket,
                             const std::unique_ptr<addrinfo *> &bind_address) {
  if (bind(server_socket, (*bind_address)->ai_addr,
           (*bind_address)->ai_addrlen)) {
    throw std::runtime_error{"bind() failed with error number " +
                             std::to_string(errno)};
  }
}

void HTTPServer::start_listening(int server_socket) {
  if (listen(server_socket, SOMAXCONN) < 0) {
    throw std::runtime_error{"listen() failed with error number " +
                             std::to_string(errno)};
  }
}

std::unique_ptr<client>
HTTPServer::getClient(std::vector<client> &client_list) {
  for (auto &client_information : client_list) {
    if (client_information.socket == -1) {
      return std::make_unique<client>(std::move(client_information));
    }
  }

  client_list.emplace_back(); // add new client at the end of the vector

  auto new_client = &client_list.back();

  new_client->address_length = sizeof(new_client->address);

  new_client->socket = -1;

  return std::make_unique<client>(*new_client);
}

void HTTPServer::logConnections() {}

std::string HTTPServer::getClientAddr() { return "localhost"; }

int HTTPServer::getSocket() { return server_socket; }

fd_set HTTPServer::waitForIncommingConnection(
    std::vector<struct client> &client_list) {

  fd_set read_fd_set;
  FD_ZERO(&read_fd_set);
  FD_SET(server_socket, &read_fd_set);
  int max_socket = server_socket;

  for (auto &client : client_list) {
    if (client.socket != -1) {
      FD_SET(client.socket, &read_fd_set);
      if (client.socket > max_socket) {
        max_socket = client.socket;
      }
    }
  }

  if (select(max_socket + 1, &read_fd_set, nullptr, nullptr, nullptr) < 0) {
    throw std::runtime_error{"select() failed with error number " +
                             std::to_string(errno)};
  }

  return read_fd_set;
}

void HTTPServer::sendResponse(struct client &client_info) {
  std::string response = "HTTP/1.1 200 OK\r\n"
                         "Content-Type: text/html; charset=UTF-8\r\n"
                         "Content-Length: 62\r\n"
                         "\r\n"
                         "<html><body><h1>Hello, World!</h1></body></html>";

  int sent = send(client_info.socket, response.c_str(), response.size(), 0);
  if (sent < 0) {
    std::cerr << "Error sending response to client: " << hstrerror(errno)
              << std::endl;
  }
  close(client_info.socket);
  client_info.socket = -1;
}
