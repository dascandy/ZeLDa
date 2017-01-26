#pragma once

#include <string>
#include <stdint.h>
#include <stddef.h>

class MmapFile {
public:
  MmapFile(const std::string& fileName);
  ~MmapFile();
  uint8_t *ptr;
  size_t length;
};



