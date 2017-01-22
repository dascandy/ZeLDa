#ifndef ELF_H
#define ELF_H

#include <stdint.h>

typedef uint32_t Elf32_Addr;
typedef uint32_t Elf32_Off;
typedef uint32_t Elf32_Word;
typedef int32_t Elf32_Sword;
typedef uint16_t Elf32_Half;
typedef uint64_t Elf32_Xword;
typedef int64_t Elf32_Sxword;

typedef uint64_t Elf64_Addr;
typedef uint64_t Elf64_Off;
typedef uint32_t Elf64_Word;
typedef int32_t Elf64_Sword;
typedef uint16_t Elf64_Half;
typedef uint64_t Elf64_Xword;
typedef int64_t Elf64_Sxword;

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
  STT_NONE = 0,
  STT_OBJECT = 1,
  STT_FUNC = 2,
  STT_SECTION = 3,
  STT_FILE = 4,
};

enum {
  SHN_UNDEF = 0,
  SHN_ABS = 0xfff1,
  SHN_COMMON = 0xfff2,
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


struct ElfHeader {
  uint32_t       ident;
  uint8_t        filclass, data_encoding, file_version;
  uint8_t        pad[9];
};

struct Elf64_Ehdr {
  uint32_t       ident;
  uint8_t        filclass, data_encoding, file_version;
  uint8_t        pad[9];
  Elf64_Half     type;
  Elf64_Half     machine;
  Elf64_Word     version;
  Elf64_Addr     entry;
  Elf64_Off      phoff;
  Elf64_Off      shoff;
  Elf64_Word     flags;
  Elf64_Half     ehsize;
  Elf64_Half     phentsize;
  Elf64_Half     phnum;
  Elf64_Half     shentsize;
  Elf64_Half     shnum;
  Elf64_Half     shstrndx;
};

struct Elf64_Shdr {
  Elf64_Word   name;
  Elf64_Word   type;
  Elf64_Xword  flags;
  Elf64_Addr   addr;
  Elf64_Off    offset;
  Elf64_Xword  size;
  Elf64_Word   link;
  Elf64_Word   info;
  Elf64_Xword  addralign;
  Elf64_Xword  entsize;
};

struct Elf64_Sym {
  Elf64_Word    name;
  uint8_t       info;
  uint8_t       other;
  Elf64_Half    shndx;
  Elf64_Addr    value;
  Elf64_Xword   size;
  uint8_t       bind() { return info >> 4; }
  uint8_t       type() { return info & 0xF; }
};

struct Elf64_Rel {
  Elf64_Addr    offset;
  Elf64_Xword   info;
  uint32_t      sym() { return info >> 8; }
  uint8_t       type() { return info & 0xFF; }
};

struct Elf64_RelA {
  Elf64_Addr    offset;
  Elf64_Xword   info;
  Elf64_Sxword  addend;
  uint32_t      sym() { return info >> 8; }
  uint8_t       type() { return info & 0xFF; }
};

struct Elf64_Phdr {
  Elf64_Word    type;
  Elf64_Off     offset;
  Elf64_Addr    vaddr;
  Elf64_Addr    paddr;
  Elf64_Word    filesz;
  Elf64_Word    memsz;
  Elf64_Word    flags;
  Elf64_Word    align;
};

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
  uint32_t      sym() { return info >> 8; }
  uint8_t       type() { return info & 0xFF; }
};

struct Elf32_RelA {
  Elf32_Addr    offset;
  Elf32_Word    info;
  Elf32_Sword   addend;
  uint32_t      sym() { return info >> 8; }
  uint8_t       type() { return info & 0xFF; }
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
  ELFCLASS32 = 1,
  ELFCLASS64 = 2,
};

struct Elf32 {
  typedef Elf32_Ehdr ElfHeader;
  typedef Elf32_Shdr SectionHeader;
  typedef Elf32_Phdr ProgramHeader;
  typedef Elf32_Sym Symbol;
  typedef Elf32_Rel Relocation;
  typedef Elf32_RelA RelocationA;
  enum { Class = ELFCLASS32 };
};

struct Elf64 {
  typedef Elf64_Ehdr ElfHeader;
  typedef Elf64_Shdr SectionHeader;
  typedef Elf64_Phdr ProgramHeader;
  typedef Elf64_Sym Symbol;
  typedef Elf64_Rel Relocation;
  typedef Elf64_RelA RelocationA;
  enum { Class = ELFCLASS64 };
};

#endif


