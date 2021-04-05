cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(USignal VERSION 0.3.0)
message(STATUS "[Project] ${PROJECT_NAME}")

include(cmake/InitUCMake.cmake)
Ubpa_InitUCMake(VERSION 0.6.4)

Ubpa_InitProject()

Ubpa_AddDep(UTemplate 0.7.2)
Ubpa_AddDep(USmallFlat 0.2.2)
Ubpa_AddDep(UFunction 0.1.0)

if(Ubpa_BuildTest_USignal)
  find_package(GTest QUIET)
  if(GTest_FOUND)
    message(NOTICE "GTest Found")
  else()
    message(NOTICE "GTest not Found, so we ignore some targets depending on GTest")
  endif()
endif()

Ubpa_AddSubDirsRec(include)
Ubpa_AddSubDirsRec(src)

Ubpa_Export(
  TARGET
  DIRECTORIES
    "include"
)
