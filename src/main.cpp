#include <stdio.h>
#include <string>
#include <vector>

int main(int argc, char **argv) 
{
  std::string outputName = "a.out";
  std::vector<std::string> inputs;

  char **arg = argv+1;
  while (*arg) {
    if (*arg == std::string("-o")) {
      arg++;
      outputName = *arg;
    } else {
      inputs.push_back(*arg);
    }
    arg++;
  }
  printf("output %s\n", outputName.c_str());
  for (const auto& n : inputs) {
    printf("input %s\n", n.c_str());
  }
}


