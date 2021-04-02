if(NOT GTest_FOUND)
  return()
endif()

set(c_options "")
if(MSVC)
  list(APPEND c_options "/wd4275")
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  #
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  #
endif()

Ubpa_AddTarget(
  TEST
  MODE EXE
  LIB
    Ubpa::USignal_core
    GTest::gtest
    GTest::gtest_main
  C_OPTION
    ${c_options}
)
