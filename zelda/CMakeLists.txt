project(zelda)

add_executable(zelda 
  main.cpp
)

target_link_libraries(${PROJECT_NAME}
  elf
)


