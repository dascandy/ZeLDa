#define _LARGEFILE64_SOURCE
#include "ElfFile.h"
#include "elf.h"
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

ElfFile::ElfFile(const std::string& fileName) {
  int fd = open(fileName.c_str(), O_RDONLY);
  length = lseek64(fd, 0, SEEK_END);
  lseek64(fd, 0, SEEK_SET);
  ptr = (uint8_t*)mmap(NULL, length, PROT_READ, MAP_PRIVATE, fd, 0);
}

ElfFile::~ElfFile() {
  munmap(ptr, length);
}

Elf32_Ehdr* ElfFile::header() {
  return reinterpret_cast<Elf32_Ehdr*>(ptr);
}

size_t ElfFile::sectioncount() {
  return header()->shnum;
}

size_t ElfFile::segmentcount() {
  return header()->phnum;
}

uint8_t *ElfFile::get(Elf32_Shdr* section) {
  return ptr + section->offset;
}

Elf32_Shdr* ElfFile::section(size_t index) {
  Elf32_Ehdr* hdr = header();
  return reinterpret_cast<Elf32_Shdr*>(ptr + hdr->shentsize * index + hdr->shoff);
}

Elf32_Phdr* ElfFile::segment(size_t index) {
  Elf32_Ehdr* hdr = header();
  return reinterpret_cast<Elf32_Phdr*>(ptr + hdr->phentsize * index + hdr->phoff);
}

const char* ElfFile::name(size_t offset) {
  return reinterpret_cast<const char*>(get(section(header()->shstrndx)) + offset);
}

const char* ElfFile::symbolname(size_t offset) {
  return reinterpret_cast<const char*>(get(section(".strtab")) + offset);
}

Elf32_Shdr* ElfFile::section(const std::string& name) {
  for (size_t index = 0; index < header()->shnum; index++) {
    if (strcmp(this->name(section(index)->name), name.c_str()) == 0) {
      return section(index);
    }
  }
  return NULL;
}

size_t ElfFile::symbolcount() {
  Elf32_Shdr *symtab = section(".symtab");
  return symtab->size / sizeof(Elf32_Sym);
}

Elf32_Sym* ElfFile::symbol(size_t index) {
  return reinterpret_cast<Elf32_Sym*>(get(section(".symtab"))) + index;
}

Elf32_Sym* ElfFile::symbol(const std::string& name) {
  Elf32_Shdr *symtab = section(".symtab");
  size_t count = symtab->size / sizeof(Elf32_Sym);
  Elf32_Sym* symbols = reinterpret_cast<Elf32_Sym*>(get(symtab));
  for (size_t index = 1; index < count; index++) {
    if (strcmp(this->name(symbols[index].name), name.c_str()) == 0) {
      return symbols + index;
    }
  }
  return NULL;
}


