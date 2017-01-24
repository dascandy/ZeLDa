#pragma once

#include <string>
#include <stdint.h>
#include <stddef.h>
#include <vector>
#include <memory>
#include <unordered_map>
#include "MmapFile.h"

class Symbol;

class Section {
public:
  enum class OutputClass {
    Code,
    RoData,
    Data,
    Bss
  };
  virtual std::string name() = 0;
  virtual void forEachRelocation(const std::function<void(Symbol*)> &cb) = 0;
  virtual OutputClass getOutputForSection() = 0;
  virtual size_t size() = 0;
  virtual size_t SetAddress(size_t address) = 0;
  virtual size_t GetAddress() = 0;
  virtual void Write(uint8_t* target, const std::unordered_map<Section*, size_t> &symbols) = 0;
};

class Symbol {
public:
  enum class Type {
    Unknown,
    Function,
    Object,
    Section,
  };
  virtual Section* section() = 0;
  virtual size_t offset() = 0;
  virtual std::string name() = 0;
  virtual Type type() = 0;
};

class ObjectFile {
public:
  virtual size_t symbolcount() = 0;
  virtual size_t sectioncount() = 0;
  virtual Section* getSection(size_t index) = 0;
  virtual Symbol* getSymbol(size_t index) = 0;
  virtual Symbol* getSymbol(const std::string& name) = 0;
};

ObjectFile* LoadElfFile(std::shared_ptr<MmapFile> file, size_t offset, size_t length);

template <typename Elf>
class ElfFile : public ObjectFile {
private:
  friend ObjectFile* LoadElfFile(std::shared_ptr<MmapFile> file, size_t offset, size_t length);
  ElfFile(std::shared_ptr<MmapFile> file, size_t offset, size_t length);
private:
  std::shared_ptr<MmapFile> file;
  uint8_t *ptr;

  struct ElfSection : Section {
    ElfSection(typename Elf::SectionHeader*, ElfFile<Elf>*);
    void forEachRelocation(const std::function<void(Symbol*)> &cb);
    OutputClass getOutputForSection();
    size_t size();
    size_t SetAddress(size_t address) override;
    size_t GetAddress() override;
    std::string name() override;
    void Write(uint8_t* , const std::unordered_map<Section*, size_t> &);
    typename Elf::SectionHeader* sec;
    ElfFile<Elf>* file;
    uint64_t addr;
  };

  struct ElfSymbol : Symbol {
    ElfSymbol(typename Elf::Symbol*, ElfFile<Elf>*);
    Section* section() override;
    size_t offset() override;
    std::string name() override;
    Type type() override;
    typename Elf::Symbol* sym;
    ElfFile<Elf>* file;
  };
  std::unordered_map<typename Elf::Symbol*, ElfSymbol> symbols;
  std::unordered_map<typename Elf::SectionHeader*, ElfSection> sections;
public:
  size_t symbolcount() override;
  size_t sectioncount() override;
  Section* getSection(size_t index) override;
  Symbol* getSymbol(size_t index) override;
  Symbol* getSymbol(const std::string& name) override;
  size_t segmentcount();
  typename Elf::ElfHeader* header();
  typename Elf::SectionHeader* section(size_t index);
  typename Elf::SectionHeader* section(const std::string& name);
  typename Elf::ProgramHeader* segment(size_t index);
  typename Elf::Symbol* symbol(size_t index);
  typename Elf::Symbol* symbol(const std::string& name);
  uint8_t *get(typename Elf::SectionHeader*);
  const char* name(size_t offset);
  const char* symbolname(size_t offset);
};

template <typename Elf>
class ElfExecutable {
public:
  ElfExecutable(const std::string& name);
  void addSegment(Section::OutputClass oclass, uint64_t vaddr, const uint8_t* data, size_t size);
  ~ElfExecutable();
private:
  std::vector<typename Elf::ProgramHeader> phdrs;
  std::string name;
  size_t offset;
  int fd;
};


