#pragma once
#include "job.h"
#include <sstream>
#include <map>
#include<string>
#include<regex>
#include<sys/stat.h>
#include "file_helper.h"
#include "tcp_send_file.h"
#include "tcp_send_string.h"
#include "http_request_header.h"
#include "http_send_dir.h"
#include "http_helper.h"
using namespace std;
class HttpParser:public Job{
private:
  char * buffer;
  const int kMaxHeaderSize = 8*1024;
  int cur_len;
  int fd;
public:
  HttpParser(TCPServer * server,int fd):Job(server){
    this->buffer = new char[kMaxHeaderSize + 1];
    this->fd = fd;
    this->cur_len = 0;
  }
  ~HttpParser(){
    delete [] this->buffer;
  }
  bool operator()(){
     while(true){
      ssize_t len = kMaxHeaderSize - cur_len;
      if(len == 0){
        //超过长度
        this->server->closeSocketNextTick(fd);
        return false;
      }
      ssize_t read_len = recv(fd, (void *)(buffer + cur_len),len, MSG_DONTWAIT | MSG_NOSIGNAL);
      if(read_len == -1){
        if(errno == EAGAIN){
          return false;
        }else{
          //fail
          this->server->closeSocketNextTick(fd);
          return false;
        }

      }else{
        if(read_len == 0){
          return false;
        }
        //try to find \r\n
        int start = 0;
        if(cur_len >= 3){
          start = cur_len - 3;
        }
        int headerEnd = -1;
        for(int pos = start; pos < cur_len + read_len - 3;pos++){
          if(buffer[pos] == '\r' && buffer[pos + 1] == '\n' && buffer[pos + 2] == '\r' && buffer[pos + 3] == '\n'){
            headerEnd = pos + 4;
            break;
          }
        }
        if(headerEnd == -1){
          //继续读
          cur_len += read_len;
          continue;
        }
        cur_len += read_len;
        onHeaderParsed(headerEnd);
        this->server->addReadJobFor(fd, new HttpParser(this->server,fd));
        return true;
      }
    }
    return true;
  }
  void writeString(const char * str){
    this->server->addWriteJobFor(fd, new TCPSendString(this->server, this->fd,str));
  }
  void onHeaderParsed(int headerEndPos){
    HttpRequestHeader request;
    map<string,string> & headers = request.headers;
    buffer[headerEndPos] = '\0';
    string line;
    istringstream header_stream(buffer);
    //get first line
    if(!getline(header_stream,line,'\r')){
      return;
    }
    istringstream req_stream(line);
    if(!(req_stream >> request.method>>request.path>>request.http_version)){
      return;
    }
    request.path = urlDecode(request.path);
    while(getline(header_stream, line,'\r')){
      istringstream iss(line);
      string part1;
      string part2;
      if(iss.peek() == '\n')
        iss.get();
      if(!getline(iss, part1,':')){
        continue;
      }
      if(!getline(iss, part2)){
        continue;
      }
      for(char & a:part1){
        a=tolower(a);
      }
      headers[part1] = part2;
    }
    handleGetRequest(request);
  }
  void handleGetRequest(HttpRequestHeader & request){
    string & path = request.path;
    path = regex_replace(path, regex("\\.\\."),"");
    path = "/mnt/c/Users/yxz/Downloads/" + path;
    cout<<path<<endl;
    struct stat file_stat;
    if(stat(path.c_str(),&file_stat) <0){
      send404();
      return;
    }
    if(isFile(file_stat)){
      int file_fd = open(path.c_str(), O_RDONLY);
      ssize_t len = getSize(file_stat);
      writeString("HTTP/1.1 200 OK\r\n");
      writeString("Content-Type: 	text/plain\r\n");
      writeString("Content-Length: ");
      writeString(to_string(len).c_str());
      writeString("\r\n\r\n");
      this->server->addWriteJobFor(fd, new TCPSendFile(this->server, fd,file_fd,0,len));
    }else if(isDir(file_stat)){
      this->server->addWriteJobFor(fd, new HttpSendDir(this->server, this->fd,path.c_str()));
    }
    else{
      send404();
      return;
    }
  }
  void send404(){
    writeString("HTTP/1.1 404 ObjectNotFound\r\nContent-Type: text/plain\r\nContent-Length: 3\r\n\r\n404");
  }
};