project(elf)

add_library(${PROJECT_NAME} STATIC
  include/elf.h
  include/ElfFile.h
  src/ElfFile.cpp
)

target_link_libraries(${PROJECT_NAME}
  PUBLIC
    file
)

target_include_directories(${PROJECT_NAME}
  PUBLIC
    include
  PRIVATE
    src
)
