#pragma once
#include <sys/stat.h>
#include <unistd.h>
bool isFile(const struct stat & file_stat){
  return S_ISREG(file_stat.st_mode);
}
bool isDir(const struct stat & file_stat){
  return S_ISDIR(file_stat.st_mode);
}
ssize_t getSize(const struct stat & file_stat){
  return file_stat.st_size;
}