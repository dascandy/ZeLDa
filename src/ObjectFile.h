#ifndef OBJECTFILE_H
#define OBJECTFILE_H

class String {

};

class Relocation {
  
};

class Section {
  std::vector<Relocation*> relocations;
};

class Symbol {
  String getName();
  Section* getSection();
};

class ObjectFile {
  std::vector<Section*> sections;
  std::vector<Symbol*> symbols;
};

#endif

