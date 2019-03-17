#pragma once
#include "job.h"
#include <unistd.h>
#include <errno.h>
#include<iostream>
using namespace std;
class TCPSendFile:public Job{
private:
  int tcp_fd;
  int file_fd;
  ssize_t pos;
  ssize_t pos_end;
  char * buffer;
  const int kBufferSize = 4096;
public:
  TCPSendFile(TCPServer * server, int tcp_fd, int file_fd ,ssize_t start, ssize_t end):Job(server){
    this->tcp_fd = tcp_fd;
    this->file_fd = file_fd;
    this->pos = start;
    this->pos_end = end;
    this->buffer = new char[kBufferSize];
  }
  bool operator()(){
    while(true){
      ssize_t len = pos_end - pos < kBufferSize ? pos_end - pos : kBufferSize;
      if(len == 0){
        return true;
      }
      ssize_t read_len = pread(file_fd, (void *)buffer,len,pos);
      if(read_len == -1){
        //fail
        this->server->closeSocketNextTick(this->tcp_fd);
        return false;
      }else{
        ssize_t write_len = send(tcp_fd,(void *) buffer, read_len, MSG_DONTWAIT | MSG_NOSIGNAL);
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
          pos += read_len;
          }
      }
    }
    return true;
  }
  ~TCPSendFile(){
    cout<<"closing fd "<<this->file_fd<<endl;
    if(this->file_fd != 0){
      close(this->file_fd);
    }
    delete [] this->buffer;
  }
};