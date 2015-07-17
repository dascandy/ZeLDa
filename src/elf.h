#ifndef ELF_H
#define ELF_H

#include <stdint.h>

typedef uint32_t Elf32_Addr;
typedef uint32_t Elf32_Off;
typedef uint32_t Elf32_Word;
typedef int32_t Elf32_Sword;
typedef uint16_t Elf32_Half;

struct Elf32_Ehdr {
  uint32_t       ident;
  uint8_t        filclass, data_encoding, file_version;
  uint8_t        pad[9];
  Elf32_Half     type;
  Elf32_Half     machine;
  Elf32_Word     version;
  Elf32_Addr     entry;
  Elf32_Off      phoff;
  Elf32_Off      shoff;
  Elf32_Word     flags;
  Elf32_Half     ehsize;
  Elf32_Half     phentsize;
  Elf32_Half     phnum;
  Elf32_Half     shentsize;
  Elf32_Half     shnum;
  Elf32_Half     shstrndx;
};

struct Elf32_Shdr {
  Elf32_Word   name;
  Elf32_Word   type;
  Elf32_Word   flags;
  Elf32_Addr   addr;
  Elf32_Off    offset;
  Elf32_Word   size;
  Elf32_Word   link;
  Elf32_Word   info;
  Elf32_Word   addralign;
  Elf32_Word   entsize;
};

struct Elf32_Sym {
  Elf32_Word    name;
  Elf32_Addr    value;
  Elf32_Word    size;
  uint8_t       info;
  uint8_t       other;
  Elf32_Half    shndx;
  uint8_t       bind() { return info >> 4; }
  uint8_t       type() { return info & 0xF; }
};

struct Elf32_Rel {
  Elf32_Addr    offset;
  Elf32_Word    info;
};

struct Elf32_RelA {
  Elf32_Addr    offset;
  Elf32_Word    info;
  Elf32_Sword   addend;
};

struct Elf32_Phdr {
  Elf32_Word    type;
  Elf32_Off     offset;
  Elf32_Addr    vaddr;
  Elf32_Addr    paddr;
  Elf32_Word    filesz;
  Elf32_Word    memsz;
  Elf32_Word    flags;
  Elf32_Word    align;
};

enum {
  ET_NONE = 0,
  ET_REL = 1,
  ET_EXEC = 2,
  ET_DYN = 3,
  ET_CORE = 4,
  ET_LOPROC = 0xff00,
  ET_HIPROC = 0xffff,
};

enum {
  SHT_NULL = 0,
  SHT_PROGBITS = 1,
  SHT_SYMTAB = 2,
  SHT_STRTAB = 3,
  SHT_RELA = 4,
  SHT_HASH = 5,
  SHT_DYNAMIC = 6,
  SHT_NOTE = 7,
  SHT_NOBITS = 8,
  SHT_REL = 9,
  SHT_SHLIB = 10,
  SHT_DYNSYM = 11,
};

enum {
  SHF_WRITE = 0x1,
  SHF_ALLOC = 0x2,
  SHF_EXECINSTR = 0x4,
};

enum {
  STT_OBJECT = 1,
  STT_FUNC = 2,
  STT_SECTION = 3,
  STT_FILE = 4,
};

enum {
  EM_NONE = 0,
  EM_386 = 3,
};

enum {
  EV_NONE = 0,
  EV_CURRENT = 1,
};

enum {
  ELFCLASS32 = 1,
  ELFCLASS64 = 2,
};

enum {
  ELFDATA2LSB = 1,
  ELFDATA2MSB = 2,
};

enum {
  PT_NULL = 0,
  PT_LOAD = 1,
  PT_DYNAMIC = 2,
  PT_INTERP = 3,
  PT_NOTE = 4,
  PT_SHLIB = 5,
  PT_PHDR = 6,
};

enum {
  R_386_NONE = 0,
  R_386_32 = 1,
  R_386_PC32 = 2,
};

#endif


