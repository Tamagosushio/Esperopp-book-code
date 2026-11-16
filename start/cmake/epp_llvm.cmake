enable_language(C)
find_package(LLVM 17 REQUIRED CONFIG)
message(STATUS "Using LLVM ${LLVM_PACKAGE_VERSION} from ${LLVM_DIR}")

separate_arguments(EPP_LLVM_DEFINITIONS NATIVE_COMMAND "${LLVM_DEFINITIONS}")

function(epp_use_llvm target)
  target_include_directories(${target} SYSTEM PRIVATE ${LLVM_INCLUDE_DIRS})
  target_compile_options(${target} PRIVATE ${EPP_LLVM_DEFINITIONS})
  if (LLVM_LINK_LLVM_DYLIB AND TARGET LLVM)
    set(libraries LLVM)
  else()
    llvm_map_components_to_libnames(libraries ${ARGN})
  endif()
  target_link_libraries(${target} PRIVATE ${libraries})
endfunction()

add_library(epp_llvm STATIC
  "${PROJECT_SOURCE_DIR}/src/codegen/llvm_ir.cc"
  "${PROJECT_SOURCE_DIR}/src/codegen/llvm_module.cc"
  "${PROJECT_SOURCE_DIR}/src/codegen/object.cc"
)

epp_use_llvm(epp_llvm Core Support native)
target_link_libraries(epp_llvm PUBLIC epp_compiler)
epp_enable_warnings(epp_llvm)

add_library(epp_driver_build STATIC
  "${PROJECT_SOURCE_DIR}/src/driver/build_executable.cc"
)
target_link_libraries(epp_driver_build PUBLIC epp_compiler PRIVATE epp_llvm)
epp_use_llvm(epp_driver_build Support)
epp_enable_warnings(epp_driver_build)
