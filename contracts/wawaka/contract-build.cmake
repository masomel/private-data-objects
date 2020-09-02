# Copyright 2020 Intel Corporation
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

IF(NOT EXISTS $ENV{PDO_HOME})
  MESSAGE(FATAL "PDO_HOME environment variable not defined")
ENDIF()

IF (NOT DEFINED ENV{WASM_MEM_CONFIG})
  MESSAGE(FATAL_ERROR "WASM_MEM_CONFIG environment variable not defined!")
ENDIF()

SET(PDO_TOP_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../")

SET(WASM_MEM_CONFIG "$ENV{WASM_MEM_CONFIG}")

# Set the memory configuration for emscripten
# LINEAR_MEMORY: Maximum size for a WASM module's linear memory (module's internal stack + static globals + padding); needs to be multiple of 64KB
# INTERNAL_STACK_SIZE: Size of a WASM module's internal data stack (part of LINEAR_MEMORY)
# sigh: clang/ld only accepts linear memory values as ints
IF (WASM_MEM_CONFIG STREQUAL "SMALL")
  SET(INTERNAL_STACK_SIZE 24672) # 24KB
  SET(LINEAR_MEMORY 65536) # 64KB
  message(STATUS "Building contracts for SMALL memory configuration")
ELSEIF (WASM_MEM_CONFIG STREQUAL "LARGE")
  SET(INTERNAL_STACK_SIZE 98304) # 96KB
  SET(LINEAR_MEMORY 262144) # 256KB
  message(STATUS "Building contracts for LARGE memory configuration")
ELSE()
  SET(INTERNAL_STACK_SIZE 49152) # 48KB
  SET(LINEAR_MEMORY 131072) # 128KB
  message(STATUS "Building contracts for MEDIUM memory configuration")
ENDIF ()

# the -O2 is actually required for the moment because it removes
# uncalled functions that clutter the wasm file
SET(CMAKE_CXX_FLAGS "-O2 -nostdlib -fPIC -fno-exceptions")
SET(CMAKE_EXECUTABLE_SUFFIX ".wasm")

SET(CONTRACT_EXPORTS "-Wl,--allow-undefined,--export=ww_dispatch,--export=ww_initialize")
SET(CMAKE_EXE_LINKER_FLAGS "-z stack-size=${INTERNAL_STACK_SIZE} -Wl,--initial-memory=${LINEAR_MEMORY},--no-threads,--no-entry ${CONTRACT_EXPORTS}")

FILE(GLOB COMMON_SOURCE common/*.cpp)

SET(LIBRARY_SOURCE ${PDO_TOP_DIR}/common/packages/parson/parson.cpp)

SET(PACKAGE_INCLUDES
  ${PDO_TOP_DIR}/contracts/wawaka/common
  ${PDO_TOP_DIR}/common/interpreter/wawaka_wasm
  ${PDO_TOP_DIR}/common/packages/parson
  ${PDO_TOP_DIR}/common)

INCLUDE_DIRECTORIES(
  ${PACKAGE_INCLUDES}
  ${WASM_SRC}/core/app-framework/base/native
  ${WASM_SRC}/core/app-framework/app-native-shared/bi-inc
  ${WASM_SRC}/core/app-framework/base/app)

SET(CONTRACT_INSTALL_DIRECTORY "$ENV{PDO_HOME}/contracts")
SET(CDI_SIGNING_KEY "cdi_compiler1_private.pem")

FUNCTION(BUILD_CONTRACT contract)
  ADD_EXECUTABLE(${contract} ${ARGN} ${COMMON_SOURCE} ${LIBRARY_SOURCE} )
  SET(b64contract ${CMAKE_CURRENT_BINARY_DIR}/_${contract}.b64)
  ADD_CUSTOM_COMMAND(
    TARGET ${contract}
    POST_BUILD
    COMMAND base64
    ARGS -w 0 ${contract}.wasm > ${b64contract})
  SET_SOURCE_FILES_PROPERTIES(${b64contract} PROPERTIES GENERATED TRUE)
  SET_DIRECTORY_PROPERTIES(PROPERTY ADDITIONAL_MAKE_CLEAN_FILES ${b64contract})
  # this can be replaced in later versions of CMAKE with target_link_properties
  SET_PROPERTY(TARGET ${contract} APPEND_STRING PROPERTY LINK_FLAGS "${CMAKE_EXE_LINKER_FLAGS}")
  INSTALL(FILES ${b64contract} DESTINATION ${CONTRACT_INSTALL_DIRECTORY})
ENDFUNCTION()

FUNCTION(BUILD_AOT_CONTRACT contract)
  MESSAGE(WARNING "!!Building wawaka contract ${contract} for experimental AoT mode. Not ready for production use!!")
  ADD_EXECUTABLE(${contract} ${ARGN} ${COMMON_SOURCE} ${LIBRARY_SOURCE} )
  SET(b64contract ${CMAKE_CURRENT_BINARY_DIR}/_${contract}.b64)
  SET(comp_report ${CMAKE_CURRENT_BINARY_DIR}/_${contract}.cdi)
  ADD_CUSTOM_COMMAND(
    TARGET ${contract}
    POST_BUILD
    COMMAND $ENV{WASM_SRC}/wamr-compiler/build/wamrc
    ARGS -sgx --format=aot -o ${contract}.aot ${contract}.wasm
    COMMAND ${PDO_TOP_DIR}/contracts/cdi/build/gen-cdi-report
    ARGS -c ${contract}.aot -k ${CDI_SIGNING_KEY} -o ${comp_report}
    COMMAND base64
    ARGS -w 0 ${contract}.aot > ${b64contract})
  SET_SOURCE_FILES_PROPERTIES(${b64contract} PROPERTIES GENERATED TRUE)
  SET_DIRECTORY_PROPERTIES(PROPERTY ADDITIONAL_MAKE_CLEAN_FILES ${b64contract})
  # this can be replaced in later versions of CMAKE with target_link_properties
  SET_PROPERTY(TARGET ${contract} APPEND_STRING PROPERTY LINK_FLAGS "${EMCC_LINK_OPTIONS}")
  INSTALL(FILES ${b64contract} ${comp_report} DESTINATION ${CONTRACT_INSTALL_DIRECTORY})
ENDFUNCTION()
