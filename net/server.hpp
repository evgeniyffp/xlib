#ifndef _CORE_NET_SERVER_H
#define _CORE_NET_SERVER_H

#include "Net.h"

#include <iostream>

#endif

#include <stdlib.h>
#include <climits>
#include <stdexcept>
#include <bitset>
#include <climits>
#include <cstring>
#include <iostream>

namespace xlib::net {
  class connection_t {
    friend class server;
  private:
    socket_t socket;
    sockaddr_in address;
    bool is_correct = true;

    connection_t(socket_t socket, sockaddr_in address)
          : socket(socket), address(address), is_correct(true) {}

  public:
    explicit operator bool() {
      return is_correct;
    }

    void close() {
      is_correct = false;
#ifdef WIN32
      closesocket(socket);
#else
      ::close(socket);
#endif
    }

    connection_t() : is_correct(false) {}

    connection_t(const connection_t&) = delete;
    connection_t& operator=(const connection_t&) = delete;

    connection_t(connection_t&&) = default; // TODO:
    connection_t& operator=(connection_t&&) = default; // TODO:
  };

  class server {
  private:
#ifdef WIN32
    WSAData wsaData;
#endif
    connection_t server_connection;

  public:
    server(const std::string& address, u_short port) {
#ifdef WIN32
      if (WSAStartup(MAKEWORD(2, 2), &this->wsaData) != 0) {
        throw std::exception("Error init Winsock\n");
      }

      if ((server_connection.socket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
#else
      if ((server_connection.socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
#endif
        throw std::runtime_error("Failed to create socket\n");

#ifndef WIN32
      int tmp = 1;
      if (setsockopt(server_connection.socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &tmp, sizeof(tmp))) {
        exit(EXIT_FAILURE);
      }
#endif
    //std::memset(&server_connection.address, 0, sizeof(server_connection.address));

      server_connection.address.sin_family = AF_INET;
      server_connection.address.sin_addr.s_addr = inet_addr(address.c_str());
      server_connection.address.sin_port = htons(port);

#ifdef WIN32
      if (bind(server_connection.socket, (struct sockaddr*)&server_connection.address, sizeof(server_connection.address)) == SOCKET_ERROR) {
        closesocket(server_connection.socket);
        throw std::exception("Failed to bind socket! WINDOWS\n");
      }

      if (listen(server_connection.socket, 3) == SOCKET_ERROR) {
        closesocket(server_connection.socket);
        throw std::exception("Failed to listen socket\n");
      }
#else
      if (bind(server_connection.socket, (struct sockaddr*)&server_connection.address, sizeof(server_connection.address)) < 0) {
        close(server_connection.socket);
        throw std::runtime_error("Failed to bind socket\n");
      }

      if (listen(server_connection.socket, 5) < 0) {
        close(server_connection.socket);
        throw std::runtime_error("Failed to listen socket\n");
      }
#endif
    }

    ~server() {
      server_connection.close();
#ifdef WIN32
      WSACleanup();
#endif
   }

    connection_t add_client() {
      connection_t client_connection;
#ifdef WIN32
      int clientAddressSize = sizeof(client_connection.address);
      if ((client_connection.socket = accept(server_connection.socket, (struct sockaddr*)&client_connection.address, &clientAddressSize)) == INVALID_SOCKET)
#else
      socklen_t clientAddressSize = sizeof(client_connection.address);
	    if ((client_connection.socket = accept(server_connection.socket, (struct sockaddr*)&client_connection.address, &clientAddressSize)) < 0)
#endif
        throw std::runtime_error("Failed to accept connection\n");
      return client_connection;
    }

    void send_data(connection_t& client_connection, std::string msg) {
#ifdef WIN32
      if (static_cast<decltype(msg)::size_type>(::send(client_connection.socket, msg.data(), msg.size() + 1, 0)) != msg.size() + 1)
#else
      if (static_cast<decltype(msg)::size_type>(::write(client_connection.socket, msg.data(), msg.size() + 1)) != msg.size() + 1)
#endif
        throw std::runtime_error("Failed to send message\n");
    }

    template <std::size_t buffer_size = MessageBuffer>
    std::string receive_data(connection_t& client_connection) {
      char buffer[buffer_size + 1] = {0};
#ifdef WIN32
      auto count_bit = ::recv(client_connection.socket, buffer, buffer_size, 0);
#else
      auto count_bit = ::read(client_connection.socket, buffer, buffer_size);
#endif
      if (count_bit > 0)
        return std::string(buffer, count_bit);
      else
        throw std::runtime_error("Failed to get an answer from client\n");
    }
  };
}

