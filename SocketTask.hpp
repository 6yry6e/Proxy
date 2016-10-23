#ifndef _SOCKET_TASK_HPP_
#define _SOCKET_TASK_HPP_

#include "ClientSocket.hpp"
#include "SocketEvent.hpp"
#include "CompletionPort.hpp"
#include "Thread.hpp"

template <typename T>
class SocketTask : public Runnable {
private:
  MYOVERLAPPED *overlapped;
  ClientSocket<T> *client;
  SocketEvent<T> *socketEvent;
  ServerSocket<T> *server;
  CompletionPort<T> *iocp;
  DWORD	bytesTransferred;
protected:
  virtual void run() {
    client->Lock();
      switch (overlapped->OperationType) {
      case CLOSE:
        socketEvent->OnClose(client, overlapped, server, iocp);
        server->Release(client);
        break;
      case ACCEPT:
        socketEvent->OnAccept(client, overlapped, server, iocp);
        break;
      case READ:
        socketEvent->OnReadFinalized(client, overlapped, bytesTransferred, server, iocp);
        break;
      case PENDING:
        socketEvent->OnPending(client, overlapped, server, iocp);
        break;
      case WRITE:
        socketEvent->OnWriteFinalized(client, overlapped, bytesTransferred, server, iocp);
        break;
      default:
       //error
        break;
      }

    client->Unlock();
    Overlapped::Release(overlapped);
  };

public:
  SocketTask(MYOVERLAPPED* overlapped, ClientSocket<T>* client, SocketEvent<T>* socketEvent, ServerSocket<T>* server, CompletionPort<T>* iocp, DWORD bytesTransferred)
    : bytesTransferred(bytesTransferred)
    , socketEvent(socketEvent)
    , overlapped(overlapped)
    , client(client)
    , server(server)
    , iocp(iocp)
  { }
};

#endif