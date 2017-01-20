#pragma once

#include <string>
#include <stdint.h>
#include <stddef.h>
#include <memory>
#include <tuple>
#include "MmapFile.h"

class ArFile {
public:
  ArFile(std::shared_ptr<MmapFile> file);
  struct iterator {
    iterator(ArFile* file, size_t offset, size_t limit);
    void operator++();
    std::tuple<std::string, size_t, size_t> operator*();
    std::string name();
    uint8_t* ptr();
    size_t offset();
    size_t length();
    bool operator==(const iterator& it);
    bool operator!=(const iterator& it);
  private:
    ArFile* file;
    size_t offset_;
    size_t limit;
  };
  iterator begin();
  iterator end();
private:
  std::shared_ptr<MmapFile> file;
};



