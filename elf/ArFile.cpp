#include "ArFile.h"
#include <string.h>

ArFile::ArFile(std::shared_ptr<MmapFile> file)
: file(file)
{}

ArFile::iterator::iterator(ArFile* file, size_t offset, size_t limit)
: file(file)
, offset_(offset)
, limit(limit)
{}

void ArFile::iterator::operator++() {
  offset_ += length() + 60;
  if (offset_ & 1) offset_++;
}

std::tuple<std::string, size_t, size_t> ArFile::iterator::operator*() {
  return std::make_tuple(name(), offset(), length());
}

std::string ArFile::iterator::name() {
  std::string fname((const char*)file->file->ptr + offset_, 16);
  while(fname.back() == ' ') fname.pop_back();
  fname.pop_back(); // No idea why there's a slash here.
  return fname;
}

uint8_t* ArFile::iterator::ptr() {
  return file->file->ptr + offset_ + 60;
}

size_t ArFile::iterator::offset() {
  return offset_ + 60;
}

size_t ArFile::iterator::length() {
  char buf[11];
  memcpy(buf, file->file->ptr + offset_ + 48, 10);
  buf[10] = 0;
  return strtoul(buf, NULL, 10);
}

bool ArFile::iterator::operator!=(const iterator& it) {
  return offset_ != it.offset_;
}

bool ArFile::iterator::operator==(const iterator& it) {
  return offset_ == it.offset_;
}

ArFile::iterator ArFile::begin() {
  return iterator(this, 8, file->length);
}

ArFile::iterator ArFile::end() {
  return iterator(this, file->length, file->length);
}


