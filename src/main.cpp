#include "ElfFile.h"
#include <stdio.h>
#include <string>
#include <vector>
#include "elf.h"
#include <unordered_map>
#include <stack>
#include <map>

int main(int argc, char **argv) 
{
  std::string outputName = "a.out";
  std::vector<ElfFile*> inputs;
  std::string entryPoint = "main";

  printf("%zu\n", sizeof(struct Elf32_Ehdr));
  char **arg = argv+1;
  while (*arg) {
    if (*arg == std::string("-o")) {
      arg++;
      outputName = *arg;
    } else if (*arg == std::string("-E")) {
      arg++;
      entryPoint = *arg;
    } else {
      inputs.push_back(new ElfFile(*arg));
    }
    arg++;
  }
  std::unordered_map<std::string, std::pair<ElfFile*, Elf32_Sym*> > symbols;
  printf("output %s\n", outputName.c_str());
  for (auto e : inputs) {
    for (size_t n = 1; n < e->symbolcount(); n++) {
      Elf32_Sym* sym = e->symbol(n);
      if (sym->type() == STT_FUNC ||
          sym->type() == STT_OBJECT)
        symbols[e->symbolname(sym->name)] = std::make_pair(e, sym);
    }
  }
  for (const auto& p : symbols) {
    printf("|%s|\n", p.first.c_str());
  }
  std::stack<std::pair<ElfFile*, Elf32_Sym*>> undef_symbols;
  if (symbols.find(entryPoint) == symbols.end()) {
    printf("Cannot find definition of entry point %s\n", entryPoint.c_str());
    exit(-1);
  }
  undef_symbols.push(symbols[entryPoint]);
  std::map<std::pair<ElfFile*, std::string>, Elf32_Shdr*> sectionsToInclude;
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
      
      // add all relocations required for these sections
      // add all new symbols from section to known set
      Elf32_Shdr* rels = s.first->section(std::string(".rel") + s.first->name(sc->name));
      if (rels) {
        Elf32_Rel* rs = (Elf32_Rel*)s.first->get(rels);
        size_t rc = rels->size / sizeof(Elf32_Rel);
        printf("%p %s %zu\n", rels, s.first->name(rels->name), rc);
        for (size_t n = 0; n < rc; n++) {
          Elf32_Rel& r = rs[n];
          Elf32_Sym* sym = s.first->symbol(r.sym());
          if (sym->shndx != SHN_UNDEF) {
            printf("found defined relocation\n");
            undef_symbols.push(std::make_pair(s.first, sym));
          } else {
            printf("found undefined symbol ref to %s\n", s.first->symbolname(sym->name));
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
        printf("%p %s %zu\n", relas, s.first->name(relas->name), rc);
        for (size_t n = 0; n < rc; n++) {
          Elf32_RelA& r = rs[n];
          Elf32_Sym* sym = s.first->symbol(r.sym());
          if (sym->shndx != SHN_UNDEF) {
            printf("found defined relocation\n");
            undef_symbols.push(std::make_pair(s.first, sym));
          } else {
            printf("found undefined symbol ref to %s\n", s.first->symbolname(sym->name));
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
}


