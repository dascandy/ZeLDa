#include "ElfFile.h"
#include "elf.h"
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>

ObjectFile* LoadElfFile(std::shared_ptr<MmapFile> file, size_t offset, size_t length) {
  ElfHeader* hdr = (ElfHeader*)(file->ptr + offset);
  if (hdr->filclass == ELFCLASS32) {
    return new ElfFile<Elf32>(file, offset, length);
  } else if (hdr->filclass == ELFCLASS64) {
    return new ElfFile<Elf64>(file, offset, length);
  } else {
    return nullptr;
  }
}

template <typename Elf>
ElfFile<Elf>::ElfFile(std::shared_ptr<MmapFile> file, size_t offset, size_t /*length*/) 
: file(file)
, ptr(file->ptr + offset)
{
}

template <typename Elf>
typename Elf::ElfHeader* ElfFile<Elf>::header() {
  return reinterpret_cast<typename Elf::ElfHeader*>(ptr);
}

template <typename Elf>
size_t ElfFile<Elf>::sectioncount() {
  return header()->shnum;
}

template <typename Elf>
size_t ElfFile<Elf>::segmentcount() {
  return header()->phnum;
}

template <typename Elf>
uint8_t *ElfFile<Elf>::get(typename Elf::SectionHeader* section) {
  return ptr + section->offset;
}

template <typename Elf>
typename Elf::SectionHeader* ElfFile<Elf>::section(size_t index) {
  typename Elf::ElfHeader* hdr = header();
  return reinterpret_cast<typename Elf::SectionHeader*>(ptr + hdr->shentsize * index + hdr->shoff);
}

template <typename Elf>
typename Elf::ProgramHeader* ElfFile<Elf>::segment(size_t index) {
  typename Elf::ElfHeader* hdr = header();
  return reinterpret_cast<typename Elf::ProgramHeader*>(ptr + hdr->phentsize * index + hdr->phoff);
}

template <typename Elf>
const char* ElfFile<Elf>::name(size_t offset) {
  return reinterpret_cast<const char*>(get(section(header()->shstrndx)) + offset);
}

template <typename Elf>
const char* ElfFile<Elf>::symbolname(size_t offset) {
  return reinterpret_cast<const char*>(get(section(".strtab")) + offset);
}

template <typename Elf>
typename Elf::SectionHeader* ElfFile<Elf>::section(const std::string& name) {
  for (size_t index = 0; index < header()->shnum; index++) {
    if (strcmp(this->name(section(index)->name), name.c_str()) == 0) {
      return section(index);
    }
  }
  return nullptr;
}

template <typename Elf>
size_t ElfFile<Elf>::symbolcount() {
  typename Elf::SectionHeader *symtab = section(".symtab");
  return symtab ? symtab->size / sizeof(typename Elf::Symbol) : 0;
}

template <typename Elf>
Section* ElfFile<Elf>::getSection(size_t index) {
  typename Elf::SectionHeader* s = section(index);
  auto it = sections.find(s);
  if (it == sections.end()) {
    sections.insert(std::pair<typename Elf::SectionHeader*, ElfSection>(s, ElfSection(s, this)));
  }
  return &sections.find(s)->second;
}

template <typename Elf>
Symbol* ElfFile<Elf>::getSymbol(size_t index) {
  typename Elf::Symbol* s = symbol(index);
  auto it = symbols.find(s);
  if (it == symbols.end()) {
    symbols.insert(std::pair<typename Elf::Symbol*, ElfSymbol>(s, ElfSymbol(s, this)));
  }
  return &symbols.find(s)->second;
}

template <typename Elf>
Symbol* ElfFile<Elf>::getSymbol(const std::string& name) {
  typename Elf::Symbol* s = symbol(name);
  auto it = symbols.find(s);
  if (it == symbols.end()) {
    symbols.insert(std::pair<typename Elf::Symbol*, ElfSymbol>(s, ElfSymbol(s, this)));
  }
  return &symbols.find(s)->second;
}

template <typename Elf>
typename Elf::Symbol* ElfFile<Elf>::symbol(size_t index) {
  return reinterpret_cast<typename Elf::Symbol*>(get(section(".symtab"))) + index;
}

template <typename Elf>
typename Elf::Symbol* ElfFile<Elf>::symbol(const std::string& name) {
  typename Elf::SectionHeader *symtab = section(".symtab");
  size_t count = symtab->size / sizeof(typename Elf::Symbol);
  typename Elf::Symbol* symbols = reinterpret_cast<typename Elf::Symbol*>(get(symtab));
  for (size_t index = 1; index < count; index++) {
    if (strcmp(this->name(symbols[index].name), name.c_str()) == 0) {
      return symbols + index;
    }
  }
  return nullptr;
}

template <typename Elf>
ElfFile<Elf>::ElfSymbol::ElfSymbol(typename Elf::Symbol* sym, ElfFile<Elf>* file) 
: sym(sym)
, file(file) 
{
}

template <typename Elf>
size_t ElfFile<Elf>::ElfSymbol::offset() {
  return sym->value;
}

template <typename Elf>
std::string ElfFile<Elf>::ElfSymbol::name() {
  return file->symbolname(sym->name);
}

template <typename Elf>
Section* ElfFile<Elf>::ElfSymbol::section() {
  return file->getSection(sym->shndx);
}

template <typename Elf>
Symbol::Type ElfFile<Elf>::ElfSymbol::type() {
  printf("%d %d\n", sym->shndx, sym->type());
  if (sym->shndx == SHN_UNDEF) 
    return Type::Unknown;
  switch (sym->type()) {
    default: 
      return Type::Unknown;
    case STT_FUNC: 
      return Type::Function;
    case STT_OBJECT: 
      return Type::Object;
    case STT_SECTION:
      return Type::Section;
  }
}

template <typename Elf>
ElfFile<Elf>::ElfSection::ElfSection(typename Elf::SectionHeader* sec, ElfFile<Elf>* file) 
: sec(sec)
, file(file)
{
}

template <typename Elf, typename Relocation>
static void forEachRelocationDo(ElfFile<Elf>* file, typename Elf::SectionHeader* rels, const std::function<void(Symbol*)> &cb) {
  Relocation* relCurrent = reinterpret_cast<Relocation*>(file->get(rels));
  Relocation* relEnd = relCurrent + rels->size / sizeof(Relocation);
  for (; relCurrent != relEnd; ++relCurrent) {
    cb(file->getSymbol(relCurrent->sym()));
  }
}

template <typename Elf>
void ElfFile<Elf>::ElfSection::forEachRelocation(const std::function<void(Symbol*)> &cb) {
  typename Elf::SectionHeader* rels = file->section(".rel" + name());
  if (rels) {
    forEachRelocationDo<Elf, typename Elf::Relocation>(file, rels, cb);
  }
  typename Elf::SectionHeader* relas = file->section(".rela" + name());
  if (relas) {
    forEachRelocationDo<Elf, typename Elf::RelocationA>(file, relas, cb);
  }
}

template <typename Elf>
std::string ElfFile<Elf>::ElfSection::name() {
  return file->name(sec->name);
}

template <typename Elf>
size_t ElfFile<Elf>::ElfSection::SetAddress(size_t address) {
  if (sec->addralign) {
    size_t mask = sec->addralign - 1;
    if (address & mask) address = (address | mask) + 1;
  }
  addr = address;
  return addr + sec->size;
}

template <typename Elf>
size_t ElfFile<Elf>::ElfSection::GetAddress() {
  return addr;
}

template <typename Elf>
Section::OutputClass ElfFile<Elf>::ElfSection::getOutputForSection() {
  if (sec->flags & SHF_EXECINSTR)
    return OutputClass::Code;
  if (sec->type == SHT_NOBITS)
    return OutputClass::Bss;
  if ((sec->flags & SHF_WRITE) == 0)
    return OutputClass::RoData;
  return OutputClass::Data;
}

template <typename Elf>
size_t ElfFile<Elf>::ElfSection::size() {
  return sec->size;
}

template <typename Elf>
void ElfFile<Elf>::ElfSection::Write(uint8_t* target, const std::unordered_map<Section*, size_t> &sections) {
  (void)target; (void)sections;
}

template <typename Elf>
ElfExecutable<Elf>::ElfExecutable(const std::string& name)
: name(name)
, offset(0x1000)
{
  fd = open(name.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0755);
  offset = 0x1000;
  lseek(fd, offset, SEEK_SET);
}

template <typename Elf>
void ElfExecutable<Elf>::addSegment(Section::OutputClass oclass, uint64_t vaddr, const uint8_t* data, size_t size) {
  typename Elf::ProgramHeader hdr;

  hdr.type = PT_LOAD;
  hdr.offset = offset;
  hdr.vaddr = vaddr;
  hdr.paddr = 0;
  hdr.filesz = oclass == Section::OutputClass::Bss ? 0 : size;
  hdr.memsz = size;
  switch(oclass) {
    case Section::OutputClass::Code:   hdr.flags = PF_R | PF_X; break;
    case Section::OutputClass::RoData: hdr.flags = PF_R; break;
    case Section::OutputClass::Data:   hdr.flags = PF_R | PF_W; break;
    case Section::OutputClass::Bss:    hdr.flags = PF_R | PF_W; break;
  }
  hdr.align = 0x1000; // because we do.
  phdrs.push_back(hdr);

  write(fd, data, size);
  offset = ((offset + size - 1) | 0xFFF) + 1;
  lseek(fd, offset, SEEK_SET);
}

template <typename Elf>
ElfExecutable<Elf>::~ElfExecutable() {
  lseek(fd, 0, SEEK_SET);
  {
    typename Elf::ElfHeader hdr;
    // Required bits
    hdr.ident = 0x464C457F;
    hdr.filclass = Elf::Class;
    hdr.data_encoding = ELFDATA2LSB;
    hdr.file_version = EV_CURRENT;
    memset(hdr.pad, 0, sizeof(hdr.pad));
    hdr.type = ET_EXEC;
    hdr.machine = EM_X64;
    hdr.version = EV_CURRENT;
    hdr.phentsize = sizeof(typename Elf::ProgramHeader);
    hdr.shentsize = sizeof(typename Elf::SectionHeader);
    hdr.ehsize = sizeof(typename Elf::ElfHeader);
    hdr.flags = 0;
    hdr.shstrndx = SHN_UNDEF;
    hdr.phoff = sizeof(typename Elf::ElfHeader);
    hdr.shnum = 0;
    hdr.shoff = 0;
    hdr.phnum = phdrs.size();
    hdr.entry = 0;
    write(fd, &hdr, sizeof(hdr));
  }
  write(fd, phdrs.data(), phdrs.size() * sizeof(typename Elf::ProgramHeader));
  close(fd);
}

template class ElfExecutable<Elf32>;
template class ElfExecutable<Elf64>;
template class ElfFile<Elf32>;
template class ElfFile<Elf64>;

