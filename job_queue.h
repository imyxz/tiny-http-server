#pragma once
#include<queue>
#include"job.h"
using std::queue;
class JobQueue{
private:
  queue<Job *> jobs;
public:
  void addJob(Job * job){
    jobs.push(job); 
  }
  bool empty(){
    return jobs.empty();
  }
  void process(){
    while(!empty()){
      Job * job = jobs.front();
      bool finished = (*job)();
      if(finished){
        delete job;
        jobs.pop();
      }else{
        break;
      }
    }
  }
  virtual ~JobQueue(){
    while(!jobs.empty()){
      Job * job = jobs.front();
      delete job;
      jobs.pop();
    }
  }
};