#pragma once
#include "job.h"
#include <unistd.h>
#include <errno.h>
#include<iostream>
#include <cstring>
using namespace std;
class TCPSendString:public Job{
private:
  int tcp_fd;
  ssize_t pos;
  ssize_t pos_end;
  char * buffer;
  const int kBufferSize = 1024;
public:
  TCPSendString(TCPServer * server, int tcp_fd, const char * str):Job(server){
    this->tcp_fd = tcp_fd;
    this->pos_end = strlen(str);
    pos = 0;
    this->buffer = new char[pos_end + 1];
    strcpy(buffer,str);
  }
  bool operator()(){
    while(true){
      ssize_t len = pos_end - pos < kBufferSize ? pos_end - pos : kBufferSize;
      if(len == 0){
        return true;
      }
      ssize_t write_len = send(tcp_fd,(void *) (buffer + pos), len, MSG_DONTWAIT | MSG_NOSIGNAL);
      if(write_len == -1){
        if(errno == EAGAIN){
          //缓冲区已满
          cerr<<"buffer full"<<endl;
          return false;
        }else{
          //发送出错
          cout<<"fail"<<endl;
          this->server->closeSocketNextTick(this->tcp_fd);
          return false;
        }
      }else{
        pos += write_len;
      }
    }
    return true;
  }
  ~TCPSendString(){
    delete [] this->buffer;
  }
};