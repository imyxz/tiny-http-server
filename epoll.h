#pragma once
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <map>
class EPoll{
private:
  int epoll_fd;
  std::map<int,int> watching_events;
public:
  EPoll(){
    this->epoll_fd = epoll_create(100);
  }
  void watchReadable(int fd){
    this->addWatchEvent(fd, EPOLLIN);
  }
  void watchWritable(int fd){
    this->addWatchEvent(fd, EPOLLOUT);
  }
  void watchDisconnect(int fd){
    this->addWatchEvent(fd, EPOLLHUP | EPOLLRDHUP);
  }
  void unWatchReadable(int fd){
    this->removeWatchEvent(fd, EPOLLIN);
  }
  void unWatchWritable(int fd){
    this->removeWatchEvent(fd, EPOLLOUT);
  }
  void unWatchDisconnect(int fd){
    this->removeWatchEvent(fd, EPOLLHUP | EPOLLRDHUP);
  }
  void addWatchEvent(int fd, int to_set_event){
    int event = 0;
    if(this->watching_events.count(fd) != 0){
      event = this->watching_events[fd];
    }
    event |= to_set_event;
    this->watch(fd,event);
  }
  void removeWatchEvent(int fd, int to_del_event){
    int event = 0;
    if(this->watching_events.count(fd) != 0){
      event = this->watching_events[fd];
    }
    event ^= to_del_event;
    this->watch(fd,event);
  }
  void unWatch(int fd){
    this->watch(fd, 0);
  }
  void watch(int fd, int events){
    epoll_event event;
    event.events = events;
    event.data.fd = fd;
    if(events == 0){
      if(this->watching_events.count(fd) == 0){
        return;
      }
      this->watching_events.erase(fd);
      epoll_ctl(this->epoll_fd, EPOLL_CTL_DEL, fd, &event);
      return;
    }
    if(this->watching_events.count(fd) == 0){
      this->watching_events[fd] = events;
      epoll_ctl(this->epoll_fd, EPOLL_CTL_ADD, fd, &event);
    }else{
      this->watching_events[fd] = events;
      epoll_ctl(this->epoll_fd, EPOLL_CTL_MOD, fd, &event);
    }
  }
  int watiEvents(epoll_event * events, int max_event, int timeout){
    return epoll_wait(this->epoll_fd, events, max_event, timeout);
  }

};