#include "ElfFile.h"
#include <stdio.h>
#include <string>
#include <vector>
#include "elf.h"
#include <unordered_map>

int main(int argc, char **argv) 
{
  std::string outputName = "a.out";
  std::vector<ElfFile*> inputs;

  printf("%zu\n", sizeof(struct Elf32_Ehdr));
  char **arg = argv+1;
  while (*arg) {
    if (*arg == std::string("-o")) {
      arg++;
      outputName = *arg;
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
  for (auto& sym : symbols) {
    printf("Found symbol %s\n", sym.first.c_str());
  }
}


