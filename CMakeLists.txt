find_package(LLVM REQUIRED CONFIG)

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

llvm_map_components_to_libnames(llvm_libs core irreader)

add_executable(truffle your_source_files.cpp)
target_link_libraries(truffle ${llvm_libs})
