#ifndef _CLIENT_SOCKET_HPP_
#define _CLIENT_SOCKET_HPP_

#include "ptr_cmp.hpp"

#include <WinSock2.h>
#include <windows.h>
#include <mutex>
#include <set>
#include <memory>

#pragma comment(lib, "Ws2_32.lib")

#define RECV_BUFFER_EMPTY	-2

enum IO_SOCK_MODES { CLOSE, ACCEPT, READ, WRITE, PENDING };

struct MYOVERLAPPED {
  OVERLAPPED Overlapped;
  WSABUF DataBuf;
  IO_SOCK_MODES OperationType;
};

namespace Overlapped {
  namespace details {
    static std::set<std::unique_ptr<MYOVERLAPPED>, ptr_comp<MYOVERLAPPED>> pool;
  }

  static void Release(MYOVERLAPPED* overlapped) {
    auto iter = details::pool.find(overlapped);
    if (iter != details::pool.end()) {
      details::pool.erase(iter);
    }
  }

  static MYOVERLAPPED* Get() {
    return details::pool.emplace(std::make_unique<MYOVERLAPPED>()).first->get();
  }

}

template <typename T>
class ClientSocket {
private:
  SOCKET fd;
  std::mutex lock;
  sockaddr_in foreignAddrIn;
  T attachment;
protected:
public:
  ClientSocket()
    : fd(INVALID_SOCKET)
  { }

  ~ClientSocket() {
    shutdown(fd, 2);
    closesocket(fd);
  }

  SOCKET GetSocket() {
    return fd;
  }

  void Lock() {
    lock.lock();
  }
  void Unlock() {
    lock.unlock();
  }
  int Associate(SOCKET socket, sockaddr_in *psForeignAddIn) {
    int nRet = SOCKET_ERROR;

    if (socket == INVALID_SOCKET)
      return nRet;

    fd = socket;
    //TODO:
    memcpy(&foreignAddrIn, psForeignAddIn, sizeof(sockaddr_in));

    return fd;
  }

  T* GetAttachment() {
    return &attachment;
  };

  int WriteToSocket(char *pBuffer, DWORD buffSize) {
    int result = SOCKET_ERROR;
    DWORD bytes = 0;
    MYOVERLAPPED *mov = Overlapped::Get();

    if (mov != NULL) {
      mov->DataBuf.buf = pBuffer;
      mov->DataBuf.len = buffSize;
      mov->OperationType = WRITE;

      result = WSASend(fd, &mov->DataBuf, 1, &bytes, 0, (OVERLAPPED *)mov, NULL);

      if (result == SOCKET_ERROR) {
        if ((WSAGetLastError()) == WSA_IO_PENDING)
          result = 0;
      }
    }
    return result;
  }

  int ReadFromSocket(char *pBuffer, DWORD buffSize) {
    int result = 0;
    DWORD bytes = 0;
    DWORD flags = 0;
    if (buffSize > 0) {
      MYOVERLAPPED *mov = Overlapped::Get();

      if (mov != NULL) {
        mov->DataBuf.buf = pBuffer;
        mov->DataBuf.len = buffSize;
        mov->OperationType = READ;

        result = WSARecv(fd, &mov->DataBuf, 1, &bytes, &flags, (OVERLAPPED *)mov, nullptr);

        if (result == SOCKET_ERROR) {
          if ((WSAGetLastError()) == WSA_IO_PENDING)
            result = 0;
        }
      }
    }
    else result = RECV_BUFFER_EMPTY;
    return result;
  }
};

#endif
