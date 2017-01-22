#include "ElfFile.h"
#include "ArFile.h"
#include "MmapFile.h"
#include <stdio.h>
#include <string>
#include <vector>
#include "elf.h"
#include <unordered_map>
#include <stack>
#include <map>
#include <string.h>
#include <algorithm>

int main(int argc, char **argv) 
{
  std::string outputName = "a.out";
  std::vector<ObjectFile*> inputs;
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
      for (const auto& entry : f) {
        printf("entry %s: %zu %zu\n", std::get<0>(entry).c_str(), std::get<1>(entry), std::get<2>(entry));
        if (std::get<0>(entry).back() == 'o') {
          printf("added\n");
          inputs.push_back(LoadElfFile(mapfile, std::get<1>(entry), std::get<2>(entry)));
        }
      }
    } else {
      std::shared_ptr<MmapFile> mapfile = std::make_shared<MmapFile>(*arg);
      inputs.push_back(LoadElfFile(mapfile, 0, mapfile->length));
    }
    arg++;
  }

  std::unordered_map<std::string, Symbol*> symbols;
  for (auto e : inputs) {
    printf("symc = %zu\n", e->symbolcount());
    for (size_t n = 1; n < e->symbolcount(); n++) {
      Symbol* sym = e->getSymbol(n);
      auto name = sym->name();
      if (name.size() > 0 && 
          sym->type() != Symbol::Type::Unknown) {
        printf("sym %s\n", sym->name().c_str());
        if (symbols.find(sym->name()) == symbols.end())
          symbols[sym->name()] = sym;
      }
    }
  }

  if (symbols.find(entryPoint) == symbols.end()) {
    printf("Cannot find definition of entry point %s\n", entryPoint.c_str());
    exit(-1);
  }

  std::stack<Section*> sectionsToDo;
  std::vector<Section*> sectionsToInclude;
  std::map<Section::OutputClass, std::vector<Section*> > outputs;
  sectionsToDo.push(symbols[entryPoint]->section());
  while (!sectionsToDo.empty()) {
    Section* sec = sectionsToDo.top();
    sectionsToInclude.push_back(sec);
    outputs[sec->getOutputForSection()].push_back(sec);
    sec->forEachRelocation([&sectionsToDo, &symbols, &sectionsToInclude](Symbol* sym) {
      if (sym->type() == Symbol::Type::Unknown) {
        sym = symbols[sym->name()]; // swap out this undefined reference for the definition
      }

      if (std::find(sectionsToInclude.begin(), sectionsToInclude.end(), sym->section()) == sectionsToInclude.end()) {
        sectionsToDo.push(sym->section());
      }
    });
  }
  std::map<Section*, uint64_t> addresses;
  uint64_t addr = 0x8048000;
  for (auto& outclass : outputs) {
    for (auto& sec : outclass.second) {
      addr = sec->SetAddress(addr);
    }
    addr = ((addr - 1) | 0xFFF) + 1;
  }
/*
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
  */
}


