cmake_minimum_required(VERSION 3.15)

project(test VERSION 0.0.1 LANGUAGES C)

add_executable(test_darray  src/test_darray.c)
add_executable(test_hashmap src/test_hashmap.c)
add_executable(test_hashset src/test_hashset.c)