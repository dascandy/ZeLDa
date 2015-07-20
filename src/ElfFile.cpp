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

ElfExecutable::ElfExecutable(const std::string& name) 
: name(name)
{
  // Large enough for now.
  storage.reserve(1048576);
  storage.resize(sizeof(Elf32_Ehdr));
  Elf32_Ehdr* hdr = header();
  // Required bits
  hdr->ident = 0x464C457F;
  hdr->filclass = ELFCLASS32;
  hdr->data_encoding = ELFDATA2LSB;
  hdr->file_version = EV_CURRENT;
  memset(hdr->pad, 0, sizeof(hdr->pad));
  hdr->type = ET_EXEC;
  hdr->machine = EM_386;
  hdr->version = EV_CURRENT;
  hdr->phentsize = sizeof(Elf32_Phdr);
  hdr->shentsize = sizeof(Elf32_Shdr);
  hdr->ehsize = sizeof(Elf32_Ehdr);
  hdr->flags = 0;
  hdr->shstrndx = SHN_UNDEF;
  hdr->phoff = sizeof(Elf32_Ehdr);
  hdr->shnum = 0;
  hdr->shoff = 0;
  hdr->phnum = 0;
  hdr->entry = 0;
}

ElfExecutable::~ElfExecutable() {
  // TODO: patch in phdrs
  FILE* f = fopen(name.c_str(), "wb");
  fwrite(storage.data(), storage.size(), 1, f);
  fclose(f);
}

Elf32_Ehdr* ElfExecutable::header() {
  return (Elf32_Ehdr*)storage.data();
}

uint8_t *ElfExecutable::get(Elf32_Phdr* phdr) {
  printf("phdr->offset %p\n", phdr->offset);
  return storage.data() + phdr->offset;
}

Elf32_Phdr* ElfExecutable::add_phdr(size_t size, uint32_t vaddr, bool isBss) {
  size_t s = (storage.size() + 0xFFF) & 0xFFFFF000;
  size_t offs = s;
  s += size;
  storage.resize(s);
  Elf32_Ehdr* ehdr = header();
  Elf32_Phdr* hdr = (Elf32_Phdr*)(storage.data() + ehdr->phoff) + ehdr->phnum;
  ehdr->phnum++;

  hdr->type = PT_LOAD;
  hdr->offset = offs;
  hdr->vaddr = vaddr;
  hdr->paddr = 0;
  hdr->filesz = size;
  hdr->memsz = isBss ? 0 : size;
  hdr->flags = 0;
  hdr->align = 0x1000; // because we do.

  return hdr;
}


