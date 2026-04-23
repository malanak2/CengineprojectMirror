cmake_minimum_required(VERSION 4.2)
project(cengineutil)

set(CMAKE_CXX_STANDARD 26)

add_library(cengineutil SHARED FileUtil.cpp FileUtil.hpp)
set_target_properties(cengineutil PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
