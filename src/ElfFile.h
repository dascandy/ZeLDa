#ifndef ELFFILE_H
#define ELFFILE_H

#include <string>
#include <stdint.h>
#include <stddef.h>

struct Elf32_Ehdr;
struct Elf32_Shdr;
struct Elf32_Phdr;
struct Elf32_Sym;

class ElfFile {
public:
  ElfFile(const std::string& fileName);
  ~ElfFile();
private:
  uint8_t *ptr;
  size_t length;
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

#endif


