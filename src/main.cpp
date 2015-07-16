#include "ElfFile.h"
#include <stdio.h>
#include <string>
#include <vector>
#include "elf.h"

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
  printf("output %s\n", outputName.c_str());
  for (auto e : inputs) {
    static size_t index = 0;
    printf("input %zu %zu secs\n", index++, e->sectioncount());
    for (size_t n = 1; n < e->sectioncount(); n++) {
      Elf32_Shdr* sectionhdr = e->section(n);
      printf("section %s offset %8X size %8X\n", e->name(sectionhdr->name), sectionhdr->offset, sectionhdr->size);
    }
    for (size_t n = 1; n < e->symbolcount(); n++) {
      Elf32_Sym* sym = e->symbol(n);
      printf("symbol %s offset %8X section %d\n", sym->name ? e->symbolname(sym->name) : "<none>", sym->value, sym->shndx);
    }
  }
}


