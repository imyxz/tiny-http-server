#pragma once
#include "job.h"
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include<sstream>
#include <sys/types.h>
#include <dirent.h>
#include "http_send_404.h"
#include "tcp_send_string.h"
using namespace std;
struct DirItem{
  string name;
  unsigned char type;

};
const string TemplatePlaceholder = "{{DIR_JSON}}";
class HttpSendDir:public Job{
private:
  static string dir_template;
  static string getTemplate(){
    if(HttpSendDir::dir_template.size() != 0){
      return HttpSendDir::dir_template;
    }
    fstream f("template/list.html",ios::in);
    std::istreambuf_iterator<char> begin(f), end;
    dir_template = string(begin,end);
    f.close();
    return dir_template;
  }
  string dir_path;
  int tcp_fd;
public:
  HttpSendDir(TCPServer * server,int tcp_fd, const char * dir_path):Job(server){
    this->dir_path = dir_path;
    this->tcp_fd = tcp_fd;
  }
  ~HttpSendDir(){
  }
  void response404(const char * str){
    this->server->addWriteJobFor(tcp_fd, new HttpSend404(this->server, tcp_fd,str));
  }
  bool operator()(){
    DIR * dir = opendir(dir_path.c_str());
    if(dir == NULL){
      response404("No such dir");
      return true;
    }
    vector<DirItem> items;
    dirent * cur = readdir(dir);
    while(cur!=NULL){
      if(cur->d_type == DT_DIR || cur->d_type == DT_REG){
        items.emplace_back(DirItem{cur->d_name, cur->d_type});
      }
      cur = readdir(dir);
    }
    closedir(dir);
    string json = generateJson(items);
    string _template = getTemplate();
    size_t pos = _template.find(TemplatePlaceholder);
    _template = _template.replace(pos, TemplatePlaceholder.size(), json);
    stringstream header;
    header<<"HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: "
      <<_template.size()<<"\r\n\r\n";
    string header_str = header.str();
    this->server->addWriteJobFor(tcp_fd,new TCPSendString(this->server,tcp_fd,header_str.c_str()));
    this->server->addWriteJobFor(tcp_fd,new TCPSendString(this->server,tcp_fd,_template.c_str()));
    return true;
  }
  string generateJson(const vector<DirItem> & items){
    stringstream ss;
    ss<<"[";
    for(auto & item: items){
      if(item.name == "." || item.name == ".."){
        continue;
      }
      ss<<"{\"name\": \""<<item.name<<"\", \"type\": " << int(item.type)<<"},";
    }
    ss<<"]";
    return ss.str();
  }
};
string HttpSendDir::dir_template = "";