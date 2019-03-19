#include<string>
#include<map>
using std::string;
using std::map;
struct HttpRequestHeader{
  string method;
  string path;
  string http_version;
  map<string,string> headers;
};