#include "ElfFile.h"
#include <stdio.h>
#include <string>
#include <vector>
#include "elf.h"
#include <unordered_map>
#include <stack>
#include <map>
#include <string.h>

enum class OutputClass {
  OUTPUT_CODE,
  OUTPUT_RODATA,
  OUTPUT_DATA,
  OUTPUT_BSS
};

OutputClass getOutputForSection(Elf32_Shdr* section) {
  if (section->flags & SHF_EXECINSTR)
    return OutputClass::OUTPUT_CODE;
  if (section->type == SHT_NOBITS)
    return OutputClass::OUTPUT_BSS;
  if ((section->flags & SHF_WRITE) == 0)
    return OutputClass::OUTPUT_RODATA;
  return OutputClass::OUTPUT_DATA;
}

int main(int argc, char **argv) 
{
  std::string outputName = "a.out";
  std::vector<ElfFile*> inputs;
  std::string entryPoint = "_start";

  char **arg = argv+1;
  while (*arg) {
    if (*arg == std::string("-o")) {
      arg++;
      outputName = *arg;
    } else if (*arg == std::string("-E")) {
      arg++;
      entryPoint = *arg;
    } else if ((*arg)[strlen(*arg)-1] == 'a') {
      std::shared_ptr<MmapFile> mapfile = std::make_shared<MmapFile>(*arg);
      ArFile f(mapfile);
      // TODO: parse entries
      for (auto&& entry : f) {
        printf("entry %s: %zu %zu\n", std::get<0>(entry).c_str(), std::get<1>(entry), std::get<2>(entry));
        if (std::get<0>(entry).back() == 'o') { // not quite secure or anything
          printf("added\n");
          inputs.push_back(new ElfFile(mapfile, std::get<1>(entry), std::get<2>(entry)));
        }
      }
    } else {
      std::shared_ptr<MmapFile> mapfile = std::make_shared<MmapFile>(*arg);
      inputs.push_back(new ElfFile(mapfile, 0, mapfile->length));
    }
    arg++;
  }
  std::unordered_map<std::string, std::pair<ElfFile*, Elf32_Sym*> > symbols;
  for (auto e : inputs) {
    Elf32_Ehdr* eh = e->header();
    printf("%08X symc = %zu\n", eh->ident, e->symbolcount());
    for (size_t n = 1; n < e->symbolcount(); n++) {
      Elf32_Sym* sym = e->symbol(n);
      printf("sym %s %p\n", e->symbolname(sym->name), sym);
      if (sym->type() == STT_FUNC ||
          sym->type() == STT_OBJECT)
        if (symbols.find(e->symbolname(sym->name)) == symbols.end())
          symbols[e->symbolname(sym->name)] = std::make_pair(e, sym);
    }
  }
  std::stack<std::pair<ElfFile*, Elf32_Sym*>> undef_symbols;
  if (symbols.find(entryPoint) == symbols.end()) {
    printf("Cannot find definition of entry point %s\n", entryPoint.c_str());
    exit(-1);
  }
  undef_symbols.push(symbols[entryPoint]);
  std::map<std::pair<ElfFile*, std::string>, Elf32_Shdr*> sectionsToInclude;
  std::map<Elf32_Shdr*, ElfFile*> owningFile;
  std::map<OutputClass, std::vector<Elf32_Shdr*> > outputs;
  while (!undef_symbols.empty()) {
    // take first symbol
    std::pair<ElfFile*, Elf32_Sym*> s = undef_symbols.top();
    undef_symbols.pop();
    // if its section not already present, 
    Elf32_Shdr* sc = s.first->section(s.second->shndx);
    std::pair<ElfFile*, std::string> key = std::make_pair(s.first, s.first->name(sc->name));
    Elf32_Shdr*& sec = sectionsToInclude[key];
    if (!sec) {
      // add its section
      sec = sc;
      outputs[getOutputForSection(sec)].push_back(sec);
      owningFile[sec] = s.first;
      
      // add all relocations required for these sections
      // add all new symbols from section to known set
      Elf32_Shdr* rels = s.first->section(std::string(".rel") + s.first->name(sc->name));
      if (rels) {
        Elf32_Rel* rs = (Elf32_Rel*)s.first->get(rels);
        size_t rc = rels->size / sizeof(Elf32_Rel);
        for (size_t n = 0; n < rc; n++) {
          Elf32_Rel& r = rs[n];
          Elf32_Sym* sym = s.first->symbol(r.sym());
          if (sym->shndx != SHN_UNDEF) {
            undef_symbols.push(std::make_pair(s.first, sym));
          } else {
            if (symbols.find(s.first->symbolname(sym->name)) == symbols.end()) {
              printf("Cannot find definition of %s\n", s.first->symbolname(sym->name));
            } else {
              undef_symbols.push(symbols[s.first->symbolname(sym->name)]);
            }
          }
        }
      }

      Elf32_Shdr* relas = s.first->section(std::string(".rela") + s.first->name(sc->name));
      if (relas) {
        Elf32_RelA* rs = (Elf32_RelA*)s.first->get(relas);
        size_t rc = relas->size / sizeof(Elf32_RelA);
        for (size_t n = 0; n < rc; n++) {
          Elf32_RelA& r = rs[n];
          Elf32_Sym* sym = s.first->symbol(r.sym());
          if (sym->shndx != SHN_UNDEF) {
            undef_symbols.push(std::make_pair(s.first, sym));
          } else {
            if (symbols.find(s.first->symbolname(sym->name)) == symbols.end()) {
              printf("Cannot find definition of %s\n", s.first->symbolname(sym->name));
            } else {
              undef_symbols.push(symbols[s.first->symbolname(sym->name)]);
            }
          }
        }
      }
    }
  }

  std::map<Elf32_Shdr*, uint32_t> addresses;
  std::map<OutputClass, Elf32_Phdr*> phdrs;
  ElfExecutable exe(outputName);
  uint32_t curAddr = 0x8048000;
  for (auto& p : outputs) {
    size_t start = curAddr;
    for (auto& r : p.second) {
      size_t mask = r->addralign - 1;
      if (curAddr & mask)
        curAddr += r->addralign - (curAddr & mask);

      printf("including %d section %p at %08X size %8X\n", p.first, r, curAddr, r->size);
      addresses[r] = curAddr;
      curAddr += r->size;
    }
    phdrs[p.first] = exe.add_phdr(curAddr - start, start, p.first == OutputClass::OUTPUT_BSS);
    curAddr = ((curAddr - 1) & 0xFFFFF000) + 0x1000;
  }

  // Second loop so all sections have a known target address (decided in first pass) that we can now relocate to
  auto& entrysym = symbols[entryPoint];
  Elf32_Shdr* entrysec = entrysym.first->section(entrysym.second->shndx);
  exe.header()->entry = addresses[entrysec] + entrysym.second->value;
  printf("Entrypoint %08X\n", exe.header()->entry);
  for (auto& p : outputs) {
    Elf32_Phdr* phdr = phdrs[p.first];
    size_t offset = 0;
    for (auto& r : p.second) {
      size_t mask = r->addralign - 1;
      if (offset & mask)
        offset += r->addralign - (curAddr & mask);
      
      memcpy(exe.get(phdr) + offset, owningFile[r]->get(r), r->size);
      ElfFile* file = owningFile[r]; 
      Elf32_Shdr* rels = file->section(std::string(".rel") + file->name(r->name));
      if (rels) {
        Elf32_Rel* rs = (Elf32_Rel*)file->get(rels);
        size_t rc = rels->size / sizeof(Elf32_Rel);
        for (size_t n = 0; n < rc; n++) {
          Elf32_Rel& r = rs[n];
          Elf32_Sym* sym = file->symbol(r.sym());
          uint32_t addr;
          if (sym->shndx != SHN_UNDEF) {
            addr = addresses[file->section(sym->shndx)] + sym->value;
          } else {
            auto& refsym = symbols[file->symbolname(sym->name)];
            addr = addresses[refsym.first->section(refsym.second->shndx)] + refsym.second->value;
          }
          if (r.type() == R_386_PC32) {
            size_t relocaddr = phdr->vaddr + offset + r.offset;
            addr -= relocaddr;
          }
          *(uint32_t*)(exe.get(phdr) + offset + r.offset) += addr;
        }
      }

      Elf32_Shdr* relas = file->section(std::string(".rela") + file->name(r->name));
      if (relas) {
        Elf32_RelA* rs = (Elf32_RelA*)file->get(relas);
        size_t rc = relas->size / sizeof(Elf32_RelA);
        for (size_t n = 0; n < rc; n++) {
          Elf32_RelA& r = rs[n];
          Elf32_Sym* sym = file->symbol(r.sym());
          uint32_t addr;
          if (sym->shndx != SHN_UNDEF) {
            addr = addresses[file->section(sym->shndx)] + sym->value;
          } else {
            auto& refsym = symbols[file->symbolname(sym->name)];
            addr = addresses[refsym.first->section(refsym.second->shndx)] + refsym.second->value;
          }
          *(uint32_t*)(exe.get(phdr) + offset + r.offset) += addr;
        }
      }
      offset += r->size;
    }
  }
}


