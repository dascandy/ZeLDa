

project(elf)

add_library(${PROJECT_NAME} STATIC
  ArFile.cpp
  ArFile.h
  ElfFile.cpp
  ElfFile.h
  elf.h
  MmapFile.cpp
  MmapFile.h
)

target_include_directories(${PROJECT_NAME}
  PUBLIC
    .
)
