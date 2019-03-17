#pragma once
class TCPServer;
class Job{
public: 
  Job(TCPServer * server){
    this->server = server;
  }
  virtual bool operator()() = 0;
  virtual ~Job(){

  };
protected:
  TCPServer * server;
};