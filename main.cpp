#include <stdio.h>
#include <conio.h>
#include <time.h>
#include "Server.hpp"
#include <string>
#define BUFF_SIZE 1024
#define MAX_CONNECTIONS 10
#define NO_THREADS 16
#define TIME_OUT 120
#define PORT 8080


struct Attachment {
  volatile time_t lastActionTime;
  char readBuffer[BUFF_SIZE];
  std::string writeBuffer;
  

  Attachment() {};

  void ResetTime(bool toZero) {
    if (toZero) lastActionTime = 0;
    else {
      time_t	lLastActionTime;
      time(&lLastActionTime);
      lastActionTime = lLastActionTime;
    }
  };

  long GetTimeElapsed() {
    time_t tmCurrentTime;

    if (0 == lastActionTime) return 0;

    time(&tmCurrentTime);
    return (tmCurrentTime - lastActionTime);
  };
};

typedef ClientSocket<Attachment> ÑSocket;
typedef ServerSocket<Attachment> SSocket;
typedef CompletionPort<Attachment> IOCP;
typedef SocketEvent<Attachment> SockEvent;
typedef Server<Attachment> EchoServer;

class EchoEventHandler : public SockEvent {
public:
  EchoEventHandler() {};
  ~EchoEventHandler() {};

  virtual void OnClose(ÑSocket *pSocket, MYOVERLAPPED *pOverlap,
    SSocket *pServerSocket, IOCP *pHIocp) override {};

  virtual void OnPending(ÑSocket *pSocket, MYOVERLAPPED *pOverlap,
    SSocket *pServerSocket, IOCP *pHIocp) override {};

  virtual void OnAccept(ÑSocket *pSocket, MYOVERLAPPED *pOverlap,
    SSocket *pServerSocket, IOCP *pHIocp) override {
    int nRet;

    nRet = pSocket->ReadFromSocket(pSocket->GetAttachment()->readBuffer, BUFF_SIZE);
    pSocket->GetAttachment()->ResetTime(false);

    if (nRet == SOCKET_ERROR) {
      pServerSocket->Release(pSocket);
    }
  };

  virtual void OnReadFinalized(ÑSocket *pSocket, MYOVERLAPPED *pOverlap,
    DWORD dwBytesTransferred, SSocket *pServerSocket, IOCP *pHIocp) override {
    int nRet;
    char* read;
    char* write;
    pSocket->GetAttachment()->writeBuffer = std::string(pSocket->GetAttachment()->readBuffer, dwBytesTransferred);
    read = pSocket->GetAttachment()->readBuffer;
    write = &pSocket->GetAttachment()->writeBuffer.front();
    fprintf(stderr, "%s", write);

    nRet = pSocket->WriteToSocket(write, dwBytesTransferred);
    if (nRet == SOCKET_ERROR) {
      pServerSocket->Release(pSocket);
      return;
    }

    nRet = pSocket->ReadFromSocket(read, BUFF_SIZE);
    pSocket->GetAttachment()->ResetTime(false);

    if (nRet == SOCKET_ERROR) {
      pServerSocket->Release(pSocket);
      return;
    }
  };

  virtual void OnWriteFinalized(ÑSocket *pSocket, MYOVERLAPPED *pOverlap,
    DWORD dwBytesTransferred, SSocket *pServerSocket, IOCP *pHIocp) override {
  };
};

//---------------------------------------------------------------------------------
int main(int argc, char* argv[])
{
  int	nRet;
  EchoServer *echo;
  EchoEventHandler *handler;
  WSAData	wsData;

  nRet = WSAStartup(MAKEWORD(2, 2), &wsData);
  if (nRet < 0) {
    return -1;
  }

  try {
    handler = new EchoEventHandler();

    echo = new EchoServer(handler, PORT,
      100, NO_THREADS, 5);
    echo->loop();

    delete echo;
    delete handler;
  }
  catch (const char *err) {
    printf("%s\n", err);
  }
  catch (const wchar_t *err) {
    wprintf(L"%ls\n", err);
  }

  WSACleanup();
  return 0;
}

