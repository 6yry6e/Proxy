#ifndef _SERVER_HPP_
#define _SERVER_HPP_

#include "ServerSocket.hpp"
#include "CompletionPort.hpp"
#include "SocketEvent.hpp"
#include "SocketTask.hpp"

template <typename T>
class Server {
private:
  ServerSocket<T>	serverSocket;
  CompletionPort<T>	iocp;
  SocketEvent<T>	*socketEvent;
  ThreadPool threadPool;

public:
  void loop() {
    fwprintf(stderr, L"Server started\n");
    DWORD bytesTransferred = 0;
    MYOVERLAPPED *overlapped = nullptr;
    ClientSocket<T> *client = nullptr;
    int i = 0;
    while (true) {
      bytesTransferred = 0;
      serverSocket.TimeCheck(iocp);
      if (++i == 10000) {
        i = 0;
        fwprintf(stderr, L"Total number of clients: %d\n", serverSocket.numOfClients());
      }

      client = serverSocket.Accept();
      if (client != nullptr) {
        //fwprintf(stderr, L"New client Total number of clients: %d\n", serverSocket.numOfClients());
        iocp.AssociateSocket(client);
        if (!(iocp.SetAcceptMode(client))) {
          serverSocket.Release(client);
        }
        client = nullptr;
      }

      if (iocp.GetQueuedCompletionStatus(&bytesTransferred, &client, &overlapped) == 0) {
        if (::GetLastError() == WAIT_TIMEOUT) continue;

        if (overlapped != nullptr) 
          bytesTransferred = 0;
        else 
          continue;
      }

      if (client == nullptr) {
        Overlapped::Release(overlapped);
        continue;
      }

      if (bytesTransferred == 0) {
          //fwprintf(stderr, L"Client disconnected\n");
          if (!(iocp.SetCloseMode(client, overlapped))) {
            serverSocket.Release(client);
          }
        Overlapped::Release(overlapped);
      } else {
        std::unique_ptr<Runnable> task = std::make_unique<SocketTask<T>>(overlapped, client, socketEvent, &serverSocket, &iocp, bytesTransferred);
        threadPool.Push(task);
      }
    }
  }

  Server(SocketEvent<T> *pSEvent, unsigned int port, unsigned int queue, unsigned int threadN, unsigned int timeout) 
    : serverSocket(port, queue, timeout)
    , iocp(1)
    , threadPool(threadN)
    , socketEvent(pSEvent) 
  { };

  virtual ~Server() {
    threadPool.interrupt();
    threadPool.join();
  };
};

#endif