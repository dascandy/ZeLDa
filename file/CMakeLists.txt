project(file)

add_library(${PROJECT_NAME} STATIC
  include/ArFile.h
  include/MmapFile.h
  src/ArFile.cpp
  src/MmapFile.cpp
)

target_include_directories(${PROJECT_NAME}
  PUBLIC
    include
  PRIVATE
    src
)
