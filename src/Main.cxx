#include "headers/Client.hxx"
#include "headers/Server.hxx"
#include <iostream>
#include <iterator>
#include <memory>
#include <unistd.h>

int main() {
  HTTPServer server;

  std::vector<client> client_list;

  while (true) {
    fd_set read_fd_set = server.waitForIncommingConnection(client_list);

    if (FD_ISSET(server.getSocket(), &read_fd_set)) {
      std::unique_ptr<client> new_client = server.getClient(client_list);

      new_client->socket =
          accept(server.getSocket(), (struct sockaddr *)&new_client->address,
                 &new_client->address_length);
      if (new_client->socket < 0) {
        std::cerr << "Error accepting client connection: " << hstrerror(errno)
                  << std::endl;
        client_list.pop_back();
      } else {
        std::cout << "Accepted new client connection" << std::endl;
        client_list.push_back(*new_client);
      }
    }

    for (auto &client : client_list) {
      if (client.socket != -1 && FD_ISSET(client.socket, &read_fd_set)) {
        client.received =
            recv(client.socket, client.request, sizeof(client.request), 0);
        if (client.received < 0) {
          std::cerr << "Error receiving data from client: " << hstrerror(errno)
                    << std::endl;
        } else if (client.received == 0) {
          std::cout << "Client disconnected" << std::endl;
          close(client.socket);
          client.socket = -1;
        } else {
          std::cout << "Received " << client.received << " bytes from client"
                    << std::endl;
          server.sendResponse(client);
        }
      }
    }
  }

  return 0;
}
