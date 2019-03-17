#include "tcp_server.h"
void TCPServer::closeSocket(int fd){
    cout<<"close " << fd<<" from tcp server"<<endl;
    socket_write_job_queue_map.erase(fd);
    socket_read_job_queue_map.erase(fd);
    this->onDisConnect(fd);
    this->client_addr_map.erase(fd);
    epoll.unWatch(fd);
    close(fd);
}
int TCPServer::start(){
    listen(this->socket_fd, 100);
    sockaddr_in client_addr;
    epoll_event tmp_event;
    epoll.watch(this->socket_fd, EPOLLIN);
    while(true){
      int event_cnt = epoll.watiEvents(this->events,kMAX_EVENT,10);
      for(int i=0;i<event_cnt;i++){
        epoll_event & cur_event = this->events[i];
        logEvent(cur_event);
        if(cur_event.data.fd == this->socket_fd){
          socklen_t addr_len = 0;
          int connect_fd = accept(this->socket_fd,(struct sockaddr *) &client_addr, &addr_len);
          epoll.watchDisconnect(connect_fd);
          socket_write_job_queue_map.emplace(connect_fd, JobQueue());
          socket_read_job_queue_map.emplace(connect_fd, JobQueue());
          this->onConnect(connect_fd);
        }else{
          if(cur_event.events & EPOLLIN){
            this->onReadable(cur_event);
          }
          if(cur_event.events & EPOLLOUT){
            this->onWritable(cur_event);
          }
          if(cur_event.events & EPOLLHUP || cur_event.events & EPOLLRDHUP){
            this->closeSocketNextTick(cur_event.data.fd);
          }        
        }
      }
      processCloseSocketQueue();
    }
  }

void TCPServer::onWritable(epoll_event event){
    int fd = event.data.fd;
    JobQueue & jobs = socket_write_job_queue_map[fd];
    if(jobs.empty()){
      epoll.unWatchWritable(fd);
      return;
    }
    jobs.process();
  }
void TCPServer::onReadable(epoll_event event){
    int fd = event.data.fd;
    JobQueue & jobs = socket_read_job_queue_map[fd];
    if(jobs.empty()){
      epoll.unWatchReadable(fd);
      return;
    }
    jobs.process();
}
void TCPServer::addReadJobFor(int fd, Job * job){
    JobQueue & jobs = socket_read_job_queue_map[fd];
    jobs.addJob(job);
    epoll.watchReadable(fd);
}
void TCPServer::addWriteJobFor(int fd, Job * job){
    JobQueue & jobs = socket_write_job_queue_map[fd];
    jobs.addJob(job);
    epoll.watchWritable(fd);
}