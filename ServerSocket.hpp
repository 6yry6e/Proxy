#ifndef _SERVER_SOCKET_HPP_
#define _SERVER_SOCKET_HPP_

#include "ClientSocket.hpp"
#include "CompletionPort.hpp"
#include "ptr_cmp.hpp"
#include <set>
#include <memory>

template <typename T>
class ServerSocket {
private:
  SOCKET	fd;
  std::mutex lock;
  /*TODO make chrono*/
  const long timeout;
  std::set<std::unique_ptr<ClientSocket<T>>, ptr_comp<ClientSocket<T>>> clients;
  std::set<ClientSocket<T>*> clientsToDelete;

public:
  //TODO temp
  int numOfClients() {
    lock.lock();
    int result = clients.size();
    lock.unlock();
    return result;
  }
  
  ServerSocket(unsigned int port, unsigned int queue, long timeout = 15 * 60 * 1000)
    : fd(INVALID_SOCKET)
    , timeout(timeout)
  {
    unsigned long lngMode = 1;
    sockaddr_in addrIn;

    // Create the server socket, set the necessary 
    // parameters for making it IOCP compatible.
    fd = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
    if (fd == INVALID_SOCKET) {
      throw "server socket creation failed.";
    }
    if (ioctlsocket(fd, FIONBIO, (u_long FAR*) &lngMode) == SOCKET_ERROR) {
      closesocket(fd);
      throw "server socket creation failed.";
    }
    addrIn.sin_family = AF_INET;
    addrIn.sin_port = htons(port);
    addrIn.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(fd, (sockaddr*)&addrIn, sizeof(addrIn)) == SOCKET_ERROR) {
      closesocket(fd);
      throw "server socket failed to bind on given port.";
    }

    // Set server socket in listen mode and set the listen queue to 20.
    if (listen(fd, queue) == SOCKET_ERROR) {
      closesocket(fd);
      throw "server scket failed to listen.";
    }
  }

  ClientSocket<T>* Accept() {
    SOCKET socket;
    sockaddr_in foreignAddrIn;
    ClientSocket<T>* client = nullptr;
    int size = sizeof(foreignAddrIn);
    socket = WSAAccept(fd, (sockaddr *)&foreignAddrIn, &size, NULL, NULL);

    if (socket != INVALID_SOCKET) {
      client = clients.emplace(std::make_unique<ClientSocket<T>>()).first->get();
      client->Associate(socket, &foreignAddrIn);
    }
    return client;
  }

  void TimeCheck(CompletionPort<T>& iocp) {
    lock.lock();
    for (auto& client : clientsToDelete) {
      auto iter = clients.find(client);
      if (iter != clients.end()) {
        clients.erase(iter);
      }
    }
    clientsToDelete.clear();
    lock.unlock();
    for (auto& client : clients) {
      client->Lock();
        if (clientsToDelete.find(client.get()) == clientsToDelete.end()) {
          long timeElapsed = client->GetAttachment()->GetTimeElapsed();
          if (timeElapsed > timeout) {
            client->GetAttachment()->ResetTime(true);
            //fwprintf(stderr, L"Client timedout\n");
            iocp.SetCloseMode(client.get());
          }
        }
      client->Unlock();
    }
  }

  void Release(ClientSocket<T>* socket) {
    if (socket) {
      lock.lock();
      clientsToDelete.insert(socket);
      lock.unlock();
    }
  }
  ~ServerSocket() {
    shutdown(fd, 2);
    closesocket(fd);
  }
};

#endif