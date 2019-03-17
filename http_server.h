#pragma once
#include <fcntl.h>
#include "tcp_server.h"
#include "tcp_send_file.h"
#include "http_parser.h"
#include <map>
using std::map;
class HttpServer:public TCPServer{
private:
public:
  HttpServer(unsigned short port):TCPServer(port){

  }
  ~HttpServer(){

  }
  void closeSocket(int fd){
    cout<<"close " << fd<<" from http server"<<endl;
    TCPServer::closeSocket(fd);
  }
  void onConnect(int fd){
    addReadJobFor(fd, new HttpParser(this, fd));
  }
};