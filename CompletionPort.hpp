#ifndef _COMPLETION_PORT_HPP_
#define _COMPLETION_PORT_HPP_

#include "ClientSocket.hpp"

#pragma comment(lib, "Ws2_32.lib")

template <typename T>
class CompletionPort {
public:
  CompletionPort(DWORD dwTimeout = 0) {
    timeout = dwTimeout;
    iocp = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
    if (iocp == NULL) {
      throw "CreateIoCompletionPort() failed to create IO Completion port.";
    }
  }

  ~CompletionPort() {
    ::CloseHandle(iocp);
  }

  void AssociateSocket(ClientSocket<T>* socket) {
    if (socket) {
      //TODO: create SOCKET operator for ClientSocket
      ::CreateIoCompletionPort((HANDLE)socket->GetSocket(), iocp, (DWORD)socket, 0);
    }
  }

  BOOL SetPendingMode(ClientSocket<T>* socket) {
    BOOL result = false;

    if (socket) {
      MYOVERLAPPED *mov = Overlapped::Get();

      if (mov) {
        mov->OperationType = PENDING;
        mov->nSession = socket->GetSession();
        result = PostQueuedCompletionStatus(socket, mov);
        if (!result) Overlapped::Release(mov);
      }
    }
    return result;
  }

  BOOL SetCloseMode(ClientSocket<T>* socket, MYOVERLAPPED* overlapped = nullptr) {
    BOOL result = false;

    if (socket) {
      MYOVERLAPPED *mov = Overlapped::Get();

      if (mov) {
        mov->OperationType = CLOSE;

        if (overlapped) {
          mov->DataBuf.buf = overlapped->DataBuf.buf;
          mov->DataBuf.len = overlapped->DataBuf.len;
        }

        result = PostQueuedCompletionStatus(socket, mov);
        if (!result) Overlapped::Release(mov);
      }
    }

    return result;
  }

  BOOL SetAcceptMode(ClientSocket<T>* socket) {
    BOOL result = false;

    if (socket) {
      MYOVERLAPPED *mov = Overlapped::Get();
      if (mov) {
        mov->OperationType = ACCEPT;
        result = PostQueuedCompletionStatus(socket, mov);
        if (!result) Overlapped::Release(mov);
      }
    }

    return result;
  }

  BOOL GetQueuedCompletionStatus(LPDWORD bytesTransferred, ClientSocket<T>** socket, MYOVERLAPPED** overlapped) {
    return ::GetQueuedCompletionStatus(iocp, bytesTransferred, (LPDWORD)socket, (LPOVERLAPPED*)overlapped, timeout);
  };
protected:
  BOOL PostQueuedCompletionStatus(ClientSocket<T>* socket, MYOVERLAPPED* overlapped) {
    return ::PostQueuedCompletionStatus(iocp, 1, (DWORD)socket, (OVERLAPPED*)overlapped);
  }
private:
  HANDLE	iocp;
  DWORD	timeout;
};

#endif