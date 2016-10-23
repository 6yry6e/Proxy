#ifndef _SOCKET_EVENT_HPP_
#define _SOCKET_EVENT_HPP_
#include "ClientSocket.hpp"
#include "ServerSocket.hpp"
#include "CompletionPort.hpp"

template<class T>
class SocketEvent {
public:
  virtual void OnClose(ClientSocket<T> *pSocket,
    MYOVERLAPPED *pOverlap,
    ServerSocket<T> *pServerSocket,
    CompletionPort<T>* iocp
    ) = 0;

  virtual void OnAccept(ClientSocket<T> *pSocket,
    MYOVERLAPPED *pOverlap,
    ServerSocket<T> *pServerSocket,
    CompletionPort<T>* iocp
    ) = 0;

  virtual void OnPending(ClientSocket<T> *pSocket,
    MYOVERLAPPED *pOverlap,
    ServerSocket<T> *pServerSocket,
    CompletionPort<T>* iocp
    ) = 0;

  virtual void OnReadFinalized(ClientSocket<T> *pSocket,
    MYOVERLAPPED *pOverlap,
    DWORD dwBytesTransferred,
    ServerSocket<T> *pServerSocket,
    CompletionPort<T> *pHIocp
    ) = 0;

  virtual void OnWriteFinalized(ClientSocket<T> *pSocket,
    MYOVERLAPPED *pOverlap,
    DWORD dwBytesTransferred,
    ServerSocket<T> *pServerSocket,
    CompletionPort<T> *pHIocp
    ) = 0;
};

#endif