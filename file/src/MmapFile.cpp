#define _LARGEFILE64_SOURCE
#include "ArFile.h"
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

MmapFile::MmapFile(const std::string& fileName) {
  int fd = open(fileName.c_str(), O_RDONLY);
  length = lseek64(fd, 0, SEEK_END);
  lseek64(fd, 0, SEEK_SET);
  ptr = (uint8_t*)mmap(NULL, length, PROT_READ, MAP_PRIVATE, fd, 0);
  fprintf(stderr, "%p == %s\n", (void*)this, fileName.c_str());
}

MmapFile::~MmapFile() {
  fprintf(stderr, "%p == dead\n", (void*)this);
  munmap(ptr, length);
}


