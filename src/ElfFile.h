#ifndef ELFFILE_H
#define ELFFILE_H

#include <string>
#include <stdint.h>
#include <stddef.h>
#include <vector>
#include <memory>
#include <tuple>

struct Elf32_Ehdr;
struct Elf32_Shdr;
struct Elf32_Phdr;
struct Elf32_Sym;

class MmapFile {
public:
  MmapFile(const std::string& fileName);
  ~MmapFile();
  uint8_t *ptr;
  size_t length;
};

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

class ElfFile {
public:
  ElfFile(std::shared_ptr<MmapFile> file, size_t offset, size_t length);
private:
  std::shared_ptr<MmapFile> file;
  uint8_t *ptr;
public:
  Elf32_Ehdr* header();
  size_t symbolcount();
  size_t sectioncount();
  size_t segmentcount();
  uint8_t *get(Elf32_Shdr*);
  Elf32_Shdr* section(size_t index);
  Elf32_Shdr* section(const std::string& name);
  Elf32_Phdr* segment(size_t index);
  Elf32_Sym* symbol(size_t index);
  Elf32_Sym* symbol(const std::string& name);
  const char* name(size_t offset);
  const char* symbolname(size_t offset);
};

class ElfExecutable {
public:
  ElfExecutable(const std::string& name);
  ~ElfExecutable();
  Elf32_Ehdr* header();
  Elf32_Phdr* add_phdr(size_t size, uint32_t vaddr, bool isBss);
  uint8_t *get(Elf32_Phdr*);
private:
  std::vector<uint8_t> storage;
  std::vector<Elf32_Phdr> phdrs;
  std::string name;
};

#endif


