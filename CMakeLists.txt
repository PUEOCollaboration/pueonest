cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
project(pueonest)

set(CMAKE_BUILD_TYPE "RelWithDebInfo")
list(APPEND CMAKE_PREFIX_PATH $ENV{ROOTSYS})
find_package(ROOT COMPONENTS Matrix Minuit REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(CFITSIO REQUIRED cfitsio) 

set(libname pueonest)
set(DICTNAME G__${libname})
if(DEFINED ENV{PUEO_INSTALL_DIR})
      message("PUEO_INSTALL_DIR is set to $ENV{PUEO_INSTALL_DIR}")
      set(UTIL_LIB_DIR $ENV{PUEO_INSTALL_DIR}/lib)
      set(UTIL_INC_DIR $ENV{PUEO_INSTALL_DIR}/include)
      set(UTIL_BIN_DIR $ENV{PUEO_INSTALL_DIR}/bin)
      set(UTIL_SHARE_DIR $ENV{PUEO_INSTALL_DIR}/share)
      set(LD_UTIL $ENV{PUEO_INSTALL_DIR}/lib)
      set(INC_UTIL $ENV{PUEO_INSTALL_DIR}/include)
  else()
message("Will install to /usr/local  PUEO_INSTALL_DIR not set")
      set(UTIL_LIB_DIR /usr/local/lib)
      set(UTIL_INC_DIR /usr/local/include)
      set(UTIL_BIN_DIR /usr/local/bin)
      set(UTIL_SHARE_DIR /usr/local/share)
      set(LD_UTIL /usr/local/lib)
      set(INC_UTIL /usr/local/include)
endif()


include(${ROOT_USE_FILE} )
include_directories( ${ROOT_INCLUDE_DIRS} ${PROJECT_SOURCE_DIR}/src ${UTIL_INC_DIR})


set(H_FILES 
    pueo/nest/fits-writer.hh 
)

set(CC_FILES
  src/fits-writer.cc
)

ROOT_GENERATE_DICTIONARY(${DICTNAME} ${H_FILES} LINKDEF src/LinkDef.h) 
add_library(${libname} SHARED ${CC_FILES} ${DICTNAME}.cxx)
target_link_libraries(${libname} ${CFITSIO_LIBRARIES})
target_include_directories(${libname} PUBLIC ${CFITSIO_INCLUDE_DIRS})
target_compile_options(${libname} PUBLIC ${CFITSIO_CFLAGS_OTHER})



install (TARGETS ${libname}
  ARCHIVE DESTINATION ${UTIL_LIB_DIR}
  LIBRARY DESTINATION ${UTIL_LIB_DIR}
  RUNTIME DESTINATION ${UTIL_BIN_DIR}
)




