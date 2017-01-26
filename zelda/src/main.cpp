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

int main(int, char **argv) 
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
        auto& name = std::get<0>(entry);
        if (!name.empty() && name.back() == 'o') {
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
    for (size_t n = 1; n < e->symbolcount(); n++) {
      Symbol* sym = e->getSymbol(n);
      if ((sym->type() == Symbol::Type::Function ||
           sym->type() == Symbol::Type::Object) &&
          sym->name().size() > 0) {
        if (symbols.find(sym->name()) == symbols.end()) {
          printf("using %s from %p\n", sym->name().c_str(), (void*)e);
          symbols[sym->name()] = sym;
        }
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
  Symbol* entry = symbols[entryPoint];
  sectionsToDo.push(symbols[entryPoint]->section());
  while (!sectionsToDo.empty()) {
    Section* sec = sectionsToDo.top();
    sectionsToDo.pop();
    if (std::find(sectionsToInclude.begin(), sectionsToInclude.end(), sec) != sectionsToInclude.end()) continue;
    printf("Adding section %s\n", sec->name().c_str());
    sectionsToInclude.push_back(sec);
    outputs[sec->getOutputForSection()].push_back(sec);
    sec->forEachRelocation([&sectionsToDo, &symbols, &sectionsToInclude](Symbol* sym) {
      Symbol* realsym = sym;
      printf("Found relocation to %s\n", sym->name().c_str());
      if (sym->type() == Symbol::Type::Unknown) {
        printf("Local one is undefined, looking for other one\n");
        realsym = symbols[sym->name()]; // swap out this undefined reference for the definition
      }
      if (!realsym) {
        printf("Unresolved relocation to %s\n", sym->name().c_str());
        exit(-1);
      }
      printf("Relocated to section %s\n", realsym->section()->name().c_str());
      sectionsToDo.push(realsym->section());
    });
  }
  std::unordered_map<Section::OutputClass, std::pair<uint64_t, uint64_t>> sizes;
  uint64_t addr = 0x8048000;
  for (auto& outclass : outputs) {
    uint64_t start = addr;
    for (auto& sec : outclass.second) {
      addr = sec->SetAddress(addr);
    }
    sizes[outclass.first] = std::make_pair(start, addr - start);
    addr = ((addr - 1) | 0xFFF) + 1;
  }
  {
    ElfExecutable<Elf64> exe("a.out");
    exe.SetEntry(entry);
    for (auto& outclass : outputs) {
      fprintf(stderr, "%d %zu %zu\n", (int)outclass.first, sizes[outclass.first].first, sizes[outclass.first].second);
      if (outclass.first == Section::OutputClass::Bss) {
        exe.addSegment(outclass.first, sizes[outclass.first].first, nullptr, sizes[outclass.first].second);
      } else {
        std::vector<uint8_t> storage;
        storage.resize(sizes[outclass.first].second);
        for (auto& sec : outclass.second) {
          sec->Write(storage.data() + sec->GetAddress() - sizes[outclass.first].first, symbols);
        }
        exe.addSegment(outclass.first, sizes[outclass.first].first, storage.data(), storage.size());
      }
    }
  }
}


