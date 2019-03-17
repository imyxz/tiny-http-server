#pragma once
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <arpa/inet.h>

#include <map>
#include <set>
#include <string>
#include <unistd.h>
#include <iostream>
#include "job_queue.h"
#include "epoll.h"
using namespace std;
const int kMAX_EVENT = 100;
const int kBufferSize = 10;
class TCPServer {
protected:
  int socket_fd;
  epoll_event * events;
  map<int, sockaddr_in> client_addr_map;
  set<int> to_close_sockets;
  EPoll epoll;
  map<int, JobQueue> socket_write_job_queue_map;
  map<int, JobQueue> socket_read_job_queue_map;
public:
  void logEvent(epoll_event event){
    int fd = getEventFd(event);
    int events = event.events;
    string type;
    if(event.events & EPOLLIN){
      type = type + "EPOLLIN|";
    }
    if(event.events & EPOLLOUT){
      type = type + "EPOLLOUT|";
    }
    if(event.events & EPOLLHUP || event.events & EPOLLRDHUP){
      type = type + "EPOLLHUP|";
    }
    cout<<getFdAddress(fd)<<":"<<type<<endl;
  }
  virtual void closeSocket(int fd);
  void closeSocketNextTick(int fd){
    cout<<"insert "<<fd<<endl;
    to_close_sockets.insert(fd);
  }
  void processCloseSocketQueue(){
    for(int fd:to_close_sockets){
      closeSocket(fd);
    }
    to_close_sockets.clear();
  }
  TCPServer(unsigned short port){
    this->socket_fd = socket(AF_INET,SOCK_STREAM,0);
    sockaddr_in addr;
    addr.sin_family=AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htons(INADDR_ANY);
    int bind_ret = bind(this->socket_fd, (struct sockaddr *)&addr, sizeof(addr));
    events = new epoll_event[kMAX_EVENT];
  }
  virtual ~TCPServer(){
    delete [] this->events;
  }
  int start();
  inline int getEventFd(const epoll_event & event) const{
    return event.data.fd;
  }
  inline in_port_t getFdPort(int fd) {
    return this->client_addr_map[fd].sin_port;
  }
  inline string getFdAddress(int fd){
    string tmp;
    tmp = inet_ntoa(this->client_addr_map[fd].sin_addr );
    return tmp;
  }
  virtual void onConnect(int fd){
    cout<<"connect: "<<fd<<" "<<this->client_addr_map[fd].sin_port<<endl;
  }
  virtual void onDisConnect(int fd){
  }
  virtual void onWritable(epoll_event event);
  virtual void onReadable(epoll_event event);
  void addReadJobFor(int fd, Job * job);
  void addWriteJobFor(int fd, Job * job);
  
};