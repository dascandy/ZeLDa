project(zelda)

add_executable(zelda 
  src/main.cpp
)

target_link_libraries(${PROJECT_NAME}
  PRIVATE
    elf
    file
)


