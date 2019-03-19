#pragma once
#include <string>
#include <sstream>
#include <cstring>
#include "job.h"
#include "tcp_send_string.h"
using namespace std;
class HttpSend404:public Job{
private:
  TCPSendString * subjob;
public:
  HttpSend404(TCPServer * server,int tcp_fd, const char * reason):Job(server){
    stringstream ss;
    int len = strlen(reason);
    ss<<"HTTP/1.1 404 ObjectNotFound\r\nContent-Type: text/plain\r\nContent-Length: "
      <<len<<"\r\n\r\n"
      <<reason;
    string str = ss.str();
    subjob = new TCPSendString(server,tcp_fd, str.c_str());
  }
  ~HttpSend404(){
    delete subjob;
  }
  bool operator()(){
    return (*subjob)();
  }
};