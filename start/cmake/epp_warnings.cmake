function(epp_enable_warnings target_name)
  if(MSVC)
    target_compile_options(
      "${target_name}"
      PRIVATE
        /W4
        /permissive-
    )
  elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(
      "${target_name}"
      PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )
  endif()
endfunction()
