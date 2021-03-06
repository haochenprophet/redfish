cmake_minimum_required(VERSION 2.6.4 FATAL_ERROR)

# The name of our project is "Redfish". CMakeLists files in this project can
# refer to the root source directory of the project as ${REDFISH_SOURCE_DIR} and
# to the root binary directory of the project as ${REDFISH_BINARY_DIR}.
project(redfish)

enable_testing()

# Define "make check" as an alias for "make test."
add_custom_target(check COMMAND ctest)

SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "The Redfish Distributed Filesystem")
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.txt")
SET(CPACK_PACKAGE_VERSION_MAJOR "1")
SET(CPACK_PACKAGE_VERSION_MINOR "0")
SET(CPACK_PACKAGE_VERSION_PATCH "0")
SET(CPACK_PACKAGE_CONTACT "cmccabe@alumni.cmu.edu")
SET(CPACK_GENERATOR "TGZ")
INCLUDE(CPack)

set(CMAKE_BUILD_TYPE, Release) # can also be Debug
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -fno-strict-aliasing")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_REENTRANT -D_FILE_OFFSET_BITS=64")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_GNU_SOURCE")
# Enable -rdynamic to get readable backtraces in the signal handlers.
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -rdynamic")
# We use pthread_key_create with a non-NULL destructor parameter, so don't
# allow unloading our shared libraries.  If we did allow this, we might get
# users into a situation where the destructor functions were invoked, but they
# had been unloaded.
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -z nodelete")


set(CMAKE_C_FLAGS_RELEASE "-O3 -Wuninitialized")
set(CMAKE_C_FLAGS_DEBUG "-g -O2 -fstack-protector -Werror")
set(CMAKE_CURRENT_BINARY_DIR, builds)
SET(CMAKE_INSTALL_PREFIX /usr)

# Although nearly all of Redfish should build and work on POSIX-compatible
# operating systems, there are a few platform-specific parts.
# Normally, we'll incorporate only the platform-specific parts appropriate for
# your operating system.  However, there is a manual override that developers
# can use for testing purposes.
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(REDFISH_DETECTED_PLATFORM "platform_linux")
else()
    set(REDFISH_DETECTED_PLATFORM "platform_posix")
endif()
set(REDFISH_PLATFORM ${REDFISH_DETECTED_PLATFORM} CACHE STRING "For developers only. Which platform to build for.")
set(REDFISH_CLIENT_LIB "fishc_real" CACHE STRING "For developers only. Which client library implementation to use (fishc_stub, fishc_hdfs, or fishc_real)")
set(REDFISH_BUILD_JNI 1 CACHE BOOL "Build Hadoop JNI bindings for Redfish")
set(REDFISH_BUILD_FUSE 1 CACHE BOOL "Build FUSE (Filesystem in Userspace) connector for Redfish")
set(REDFISH_SO_REUSEADDR_HACK 0 CACHE BOOL "For developers only. Set to TRUE to use a hack which makes running unit tests easier.")

if (REDFISH_BUILD_JNI)
    find_package(JNI REQUIRED)
endif (REDFISH_BUILD_JNI)
if (REDFISH_SO_REUSEADDR_HACK)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DREDFISH_SO_REUSEADDR_HACK=1")
endif (REDFISH_SO_REUSEADDR_HACK)

IF (${CMAKE_SYSTEM_NAME} MATCHES "Linux" AND REDFISH_BUILD_FUSE)
    # Find FUSE
    find_path(FUSE_C_INCLUDE_DIR "fuse/fuse_lowlevel.h")
    find_library(FUSE_C_LIBRARY fuse)
    IF(FUSE_C_INCLUDE_DIR AND FUSE_C_LIBRARY)
    ELSE(FUSE_C_INCLUDE_DIR AND FUSE_C_LIBRARY)
        MESSAGE(FATAL_ERROR "Failed to find FUSE.  If you don't want to build the FUSE connector, set REDFISH_BUILD_FUSE to 0.  On the other hand, to install FUSE, try installing fuse-devel with apt-get or yum.")
    ENDIF(FUSE_C_INCLUDE_DIR AND FUSE_C_LIBRARY)
ENDIF (${CMAKE_SYSTEM_NAME} MATCHES "Linux" AND REDFISH_BUILD_FUSE)

# Find libjson-c
find_path(JSON_C_INCLUDE_DIR "json/json.h")
find_library(JSON_C_LIBRARY json)
IF(JSON_C_INCLUDE_DIR AND JSON_C_LIBRARY)
ELSE(JSON_C_INCLUDE_DIR AND JSON_C_LIBRARY)
    MESSAGE(FATAL_ERROR "Failed to find libjson-c. Try installing libjson-c with apt-get or yum, or install it manually from http://oss.metaparadigm.com/json-c/")
ENDIF(JSON_C_INCLUDE_DIR AND JSON_C_LIBRARY)

# Find libev
find_path(LIBEV_INCLUDE_DIR ev.h HINTS "/usr/include" "/usr/include/libev")
find_library(LIBEV_LIBRARIES NAMES ev)
IF(LIBEV_INCLUDE_DIR AND LIBEV_LIBRARIES)
ELSE(LIBEV_INCLUDE_DIR AND LIBEV_LIBRARIES)
    MESSAGE(FATAL_ERROR "Failed to find libev. Try installing libev with apt-get or yum, or install it manually from http://software.schmorp.de/pkg/libev.html")
ENDIF(LIBEV_INCLUDE_DIR AND LIBEV_LIBRARIES)

# Find leveldb
find_path(LEVELDB_INCLUDE_DIR leveldb/c.h HINTS "/usr/include")
find_library(LEVELDB_LIBRARIES NAMES leveldb)
IF(LEVELDB_INCLUDE_DIR AND LEVELDB_LIBRARIES)
ELSE(LEVELDB_INCLUDE_DIR AND LEVELDB_LIBRARIES)
    MESSAGE(FATAL_ERROR "Failed to find levelDB. Try reading INSTALL.txt, or install leveldb manually from http://code.google.com/p/leveldb/")
ENDIF(LEVELDB_INCLUDE_DIR AND LEVELDB_LIBRARIES)

# Set up include paths
include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${JSON_C_INCLUDE_DIR}
    ${LIBEV_INCLUDE_DIR}
    ${LEVELDB_INCLUDE_DIR}
)

macro(install_stest stest)
    INSTALL(TARGETS ${stest} DESTINATION stest)
endmacro(install_stest)

macro(add_utest utest)
    add_test(${utest} ${CMAKE_CURRENT_BINARY_DIR}/${utest} ${utest})
endmacro(add_utest)

add_subdirectory(client)
add_subdirectory(common)
add_subdirectory(core)
IF (${CMAKE_SYSTEM_NAME} MATCHES "Linux" AND REDFISH_BUILD_FUSE)
    add_subdirectory(fuse)
ENDIF (${CMAKE_SYSTEM_NAME} MATCHES "Linux" AND REDFISH_BUILD_FUSE)
if (REDFISH_BUILD_JNI)
    add_subdirectory(hadoop)
endif (REDFISH_BUILD_JNI)
add_subdirectory(jorm)
add_subdirectory(mds)
add_subdirectory(mkfs)
add_subdirectory(msg)
add_subdirectory(osd)
add_subdirectory(rsem)
add_subdirectory(stest)
add_subdirectory(tool)
add_subdirectory(util)
