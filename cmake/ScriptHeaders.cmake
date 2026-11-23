# By claude
function(generate_script_header OUT_FILE)
  cmake_parse_arguments(ARG "" "" "DIRS" ${ARGN})

  set(headers "")
  foreach(dir IN LISTS ARG_DIRS)
    file(GLOB_RECURSE found CONFIGURE_DEPENDS "${dir}/*.hpp")
    list(APPEND headers ${found})
  endforeach()
  list(SORT headers)

  set(content "// AUTO-GENERATED - do not edit\n#pragma once\n")
  foreach(h IN LISTS headers)
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${h}")
    file(READ "${h}" text)
    if(text MATCHES "\\[\\[ *= *(Engine::)?(Attr::)?Script")
      string(APPEND content "#include \"${h}\"\n")
    endif()
  endforeach()

  file(GENERATE OUTPUT "${OUT_FILE}" CONTENT "${content}")
endfunction()