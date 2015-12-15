#ifndef ATOM_H
#define ATOM_H

class Relocation {
public:
  std::string atomName;
};

class Atom {
  std::vector<uint8_t> bytes;
  std::vector<Relocation> relocations;
  
};

#endif


