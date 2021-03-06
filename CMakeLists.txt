CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(libneat)

set(BUILD_MAJOR "0")
set(BUILD_MINOR "0")
set(BUILD_PATCH "1~td161")
set(BUILD_VERSION ${BUILD_MAJOR}.${BUILD_MINOR}.${BUILD_PATCH})

# INSTALLATION_DIRECTORIES
#################################################

SET(CMAKE_INSTALL_BINDIR_DOC        "user executables [PREFIX/bin]")
SET(CMAKE_INSTALL_SBINDIR_DOC       "system admin executables [EPREFIX/sbin]")
SET(CMAKE_INSTALL_DATADIR_DOC       "read-only architecture-independent data [PREFIX/share]")
SET(CMAKE_INSTALL_SYSCONFDIR_DOC    "read-only single-machine data [PREFIX/etc]")
SET(CMAKE_INSTALL_LOCALSTATEDIR_DOC "modifiable single-machine data [PREFIX/var]")
SET(CMAKE_INSTALL_LIBDIR_DOC        "object code libraries [PREFIX/lib]")
SET(CMAKE_INSTALL_INCLUDEDIR_DOC    "read-only includes [PREFIX/include]")
SET(CMAKE_INSTALL_MANDIR_DOC        "man documentation [PREFIX/share/man]")

SET(CMAKE_INSTALL_BINDIR "bin"
    CACHE PATH ${CMAKE_INSTALL_BINDIR_DOC})
SET(CMAKE_INSTALL_SBINDIR "sbin"
    CACHE PATH ${CMAKE_INSTALL_SBINDIR_DOC})
SET(CMAKE_INSTALL_DATADIR "share"
    CACHE PATH ${CMAKE_INSTALL_DATADIR_DOC})
SET(CMAKE_INSTALL_SYSCONFDIR "etc"
    CACHE PATH ${CMAKE_INSTALL_SYSCONFDIR_DOC})
SET(CMAKE_INSTALL_LOCALSTATEDIR "var"
    CACHE PATH ${CMAKE_INSTALL_LOCALSTATEDIR_DOC})
SET(CMAKE_INSTALL_LIBDIR "lib"
    CACHE PATH ${CMAKE_INSTALL_LIBDIR_DOC})
SET(CMAKE_INSTALL_INCLUDEDIR "include"
    CACHE PATH ${CMAKE_INSTALL_INCLUDEDIR_DOC})
SET(CMAKE_INSTALL_MANDIR "share/man"
    CACHE PATH ${CMAKE_INSTALL_MANDIR_DOC})

# PACKAGING
#################################################

set(CPACK_SOURCE_GENERATOR "TGZ")
set(CPACK_SOURCE_PACKAGE_FILE_NAME
   "${CMAKE_PROJECT_NAME}-${BUILD_MAJOR}.${BUILD_MINOR}.${BUILD_PATCH}")
set(CPACK_SOURCE_IGNORE_FILES
   "${PROJECT_SOURCE_DIR}/${CMAKE_PROJECT_NAME}-*;${PROJECT_SOURCE_DIR}/${CMAKE_PROJECT_NAME}_*;/build/;/.git/;/CMakeFiles/;CMakeCache.txt$;.cmake$;.tar.gz$;/_CPack_Packages;/Makefile$;~$;/build-deb;/clean-deb;/filter-empty-entries;/make-symbols;/make-ppa;/make-deb;/debian.conf;/make-rpm;/rpm.conf;${CPACK_SOURCE_IGNORE_FILES}")
include(CPack)

add_custom_target(dist COMMAND ${CMAKE_MAKE_PROGRAM} clean package_source)

INCLUDE(CheckIncludeFile)
INCLUDE(CheckStructHasMember)
INCLUDE(CheckTypeSize)
SET(CMAKE_MACOSX_RPATH 1)

# SOURCES + HEADERS
#################################################
LIST(APPEND neat_headers
    neat.h
    neat_queue.h
)

LIST(APPEND neat_sources
    neat_addr.c
    neat_core.c
    neat_he.c
    neat_log.c
    neat_qos.c
    neat_stat.c
    neat_json_helpers.c
    neat_pvd.c
    neat_resolver.c
    neat_resolver_conf.c
    neat_resolver_helpers.c
    neat_security.c
    neat_pm_socket.c
    neat_unix_json_socket.c
    tls-trust.c
)

# OS DEPENDENT
#################################################
IF (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    MESSAGE(${CMAKE_SYSTEM_NAME} " supported")
    ADD_DEFINITIONS("-D_DEFAULT_SOURCE")
    LIST(APPEND neat_headers neat_linux.h)
    LIST(APPEND neat_sources neat_linux.c)

ELSEIF (${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
    MESSAGE(${CMAKE_SYSTEM_NAME} " supported")
    LIST(APPEND neat_headers neat_bsd.h)
    LIST(APPEND neat_sources neat_bsd.c)
    SET(CMAKE_REQUIRED_INCLUDES "/usr/local/include" "/usr/include")
    SET(CMAKE_LIBRARY_PATH "/usr/local/lib")
    INCLUDE_DIRECTORIES("/usr/local/include")

ELSEIF (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    MESSAGE(${CMAKE_SYSTEM_NAME} " supported")
    LIST(APPEND neat_headers neat_bsd.h)
    LIST(APPEND neat_sources neat_bsd.c)
    SET(CMAKE_REQUIRED_INCLUDES "/usr/local/include" "/usr/include" "/usr/local/opt/openssl/include")
    SET(CMAKE_LIBRARY_PATH "/usr/local/lib")
    INCLUDE_DIRECTORIES("/usr/local/include" "/usr/local/opt/openssl/include")

ELSEIF (${CMAKE_SYSTEM_NAME} MATCHES "NetBSD")
    MESSAGE(${CMAKE_SYSTEM_NAME} " supported")
    LIST(APPEND neat_headers neat_bsd.h)
    LIST(APPEND neat_sources neat_bsd.c)
    SET(CMAKE_REQUIRED_INCLUDES "/usr/pkg/include" "/usr/include" "/usr/local/include")
    SET(CMAKE_LIBRARY_PATH "/usr/local/lib")
    INCLUDE_DIRECTORIES("/usr/pkg/include" "/usr/local/include")

ELSE()
    MESSAGE(FATAL_ERROR ${CMAKE_SYSTEM_NAME} " not supported (yet?)")

ENDIF()

find_package(Threads REQUIRED)

#################################################
# CHECK STRUCT MEMBERS
#################################################
CHECK_STRUCT_HAS_MEMBER("struct sockaddr" "sa_len" "sys/types.h;sys/socket.h" HAVE_SA_LEN)
IF (HAVE_SA_LEN)
    MESSAGE(STATUS "HAVE_SA_LEN")
    ADD_DEFINITIONS(-DHAVE_SA_LEN)
ENDIF()

CHECK_STRUCT_HAS_MEMBER("struct sockaddr_in" "sin_len" "sys/types.h;netinet/in.h" HAVE_SIN_LEN)
IF (HAVE_SIN_LEN)
    MESSAGE(STATUS "HAVE_SIN_LEN")
    ADD_DEFINITIONS(-DHAVE_SIN_LEN)
ENDIF()

CHECK_STRUCT_HAS_MEMBER("struct sockaddr_in6" "sin6_len" "sys/types.h;netinet/in.h" HAVE_SIN6_LEN)
IF (HAVE_SIN6_LEN)
    MESSAGE(STATUS "HAVE_SIN6_LEN")
    ADD_DEFINITIONS(-DHAVE_SIN6_LEN)
ENDIF()

CHECK_STRUCT_HAS_MEMBER("struct sockaddr_storage" "ss_len" "sys/types.h;sys/socket.h" HAVE_SS_LEN)
IF (HAVE_SS_LEN)
    MESSAGE(STATUS "HAVE_SS_LEN")
    ADD_DEFINITIONS(-DHAVE_SS_LEN)
ENDIF()

#################################################
# CHECK OPTIONS
#################################################
# Debug build type as default
IF (NOT CMAKE_BUILD_TYPE)
    message("No build type selected, using DEBUG")
    set(CMAKE_BUILD_TYPE "DEBUG")
    # _FORTIFY_SOURCE requires compiling with optimization
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O")
endif()

OPTION(USRSCTP_SUPPORT "Include usrsctp support" 0)
IF (USRSCTP_SUPPORT)
    ADD_DEFINITIONS(-DUSRSCTP_SUPPORT)
    ADD_DEFINITIONS(-DINET)
    ADD_DEFINITIONS(-DINET6)
    ADD_DEFINITIONS(-DSCTP_DEBUG)
    LIST(APPEND neat_headers neat_usrsctp.h)
    LIST(APPEND neat_sources neat_usrsctp.c)

    FIND_LIBRARY(USRSCTP_LIB usrsctp)
    LIST(APPEND neat_libs ${USRSCTP_LIB})
    MESSAGE("USRSCTP found: " ${USRSCTP_LIB})
ENDIF()

OPTION(NEAT_LOG "enable NEAT log module" 1)
IF (NEAT_LOG)
    ADD_DEFINITIONS(-DNEAT_LOG)
ENDIF()

OPTION(FLOW_GROUPS "Include support for flow grouping" 0)
IF (FLOW_GROUPS)
    ADD_DEFINITIONS(-DFLOW_GROUPS)
ENDIF()

OPTION(SCTP_MULTISTREAMING "Include support for SCTP multistreaming" 0)
IF (SCTP_MULTISTREAMING)
    ADD_DEFINITIONS(-DSCTP_MULTISTREAMING)
ENDIF()

OPTION(OPENSSL_SUPPORT "Include support for OpenSSL" 1)

OPTION(SOCKET_API "Include the socket API" 0)

OPTION(SWIG_PYTHON "Include SWIG Python bindings" 0)
IF (SWIG_PYTHON)
    FIND_PACKAGE(SWIG REQUIRED)
    INCLUDE(${SWIG_USE_FILE})
    FIND_PACKAGE(PythonLibs 2.7 REQUIRED)
    INCLUDE_DIRECTORIES(${PYTHON_INCLUDE_PATH})

    INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

    SET(CMAKE_SWIG_FLAGS "")
    SET(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}  -Wno-error=shadow -Wno-error=pedantic")

    #SET_SOURCE_FILES_PROPERTIES(neat_swig.i PROPERTIES CPLUSPLUS ON)
    SET_SOURCE_FILES_PROPERTIES(neat_swig.i PROPERTIES SWIG_FLAGS "-includeall")
ENDIF()

OPTION(SANITIZER_ADDRESS "Compile with address sanitizer" 0)

OPTION(SANITIZER_MEMORY "Compile with memory sanitizer" 0)

IF (SANITIZER_ADDRESS AND SANITIZER_MEMORY)
MESSAGE(FATAL_ERROR "Can not compile with both sanitizer options")
ENDIF()



# REQUIREMENTS
#################################################
CHECK_INCLUDE_FILE(uv.h HAVE_SYS_UV_H)
FIND_LIBRARY(UV_LIB uv)

IF (NOT HAVE_SYS_UV_H OR NOT UV_LIB)
    MESSAGE(FATAL_ERROR "uv.h not found - libuv installed?")
ELSE()
    MESSAGE("UV found: " ${UV_LIB})
ENDIF()

CHECK_INCLUDE_FILE(ldns/ldns.h HAVE_SYS_LDNS_H)
FIND_LIBRARY(LDNS_LIB ldns)
IF (NOT HAVE_SYS_LDNS_H OR NOT LDNS_LIB)
    MESSAGE(FATAL_ERROR "ldns/ldns.h not found - libdns installed?")
ELSE()
    MESSAGE("LDNS found: " ${LDNS_LIB})
ENDIF()

IF (OPENSSL_SUPPORT)
    FIND_PACKAGE(OpenSSL)
    IF (NOT OPENSSL_FOUND)
        MESSAGE(WARNING "openssl >= 1.0.2 required for TLS - none found")
    ELSE()
        MESSAGE("OPENSSL version found: " ${OPENSSL_VERSION})
        IF (OPENSSL_VERSION VERSION_LESS "1.0.2")
            MESSAGE(WARNING "openssl >= 1.0.2 required for TLS")
        ELSE()
            CHECK_INCLUDE_FILE(openssl/ssl.h HAVE_SYS_OPENSSL_H)
            MESSAGE(STATUS "OPENSSL Crypto found: " ${OPENSSL_LIBRARIES})
            ADD_DEFINITIONS(-DNEAT_USETLS)
            SET(CMAKE_EXTRA_INCLUDE_FILES "openssl/ssl.h")

            IF ((OPENSSL_VERSION VERSION_EQUAL "1.1.0") OR (OPENSSL_VERSION VERSION_GREATER "1.1.0"))
                CHECK_TYPE_SIZE("struct bio_dgram_sctp_sndinfo" OPENSSL_DTLS)
                IF (HAVE_OPENSSL_DTLS)
                    MESSAGE("DTLS for SCTP supported")
                    ADD_DEFINITIONS(-DNEAT_SCTP_DTLS)
                ENDIF()
            ENDIF()
            if (OPENSSL_ROOT_DIR)
                # if a custom dir was set, specify the include path too
                SET(CMAKE_C_FLAGS "-I${OPENSSL_ROOT_DIR}/include")
            ENDIF()
        ENDIF()
    ENDIF()
ENDIF()

IF (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    FIND_LIBRARY(MNL_LIB mnl)
    CHECK_INCLUDE_FILE(libmnl/libmnl.h HAVE_SYS_LIBMNL_H)
    IF(NOT HAVE_SYS_LIBMNL_H OR NOT MNL_LIB)
        MESSAGE(FATAL_ERROR "libmnl/libmnl.h not found - libmnl installed?")
    ELSE()
        MESSAGE("LIBMNL found: " ${MNL_LIB})
    ENDIF()
    FIND_LIBRARY(SCTP_LIB sctp)
    IF(EXISTS "/proc/sys/net/mptcp/mptcp_enabled")
        MESSAGE(STATUS "MPTCP found")
        ADD_DEFINITIONS(-DMPTCP_SUPPORT)
    ENDIF()
ENDIF()

CHECK_INCLUDE_FILE(jansson.h HAVE_JANSSON_H)
FIND_LIBRARY(JANSSON_LIB jansson)
IF (NOT HAVE_JANSSON_H OR NOT JANSSON_LIB)
    MESSAGE(FATAL_ERROR "jansson.h not found - jansson installed?")
ELSE()
    MESSAGE(STATUS "jansson found: " ${JANSSON_LIB})
ENDIF()

CHECK_INCLUDE_FILE(netinet/sctp.h HAVE_NETINET_SCTP_H)
IF (HAVE_NETINET_SCTP_H)
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DHAVE_NETINET_SCTP_H")
    SET(SCTP_INCLUDE "netinet/sctp.h")
ENDIF()

IF (USRSCTP_SUPPORT)
    CHECK_INCLUDE_FILE(usrsctp.h HAVE_USRSCTP_H)
    IF (NOT HAVE_USRSCTP_H)
      MESSAGE(FATAL_ERROR "usrsctp.h not found - usrsctp installed?")
    ENDIF()

    SET(SCTP_INCLUDE "usrsctp.h")
ENDIF()

IF (SCTP_INCLUDE)
    CHECK_STRUCT_HAS_MEMBER("struct sctp_event_subscribe" sctp_association_event ${SCTP_INCLUDE} HAVE_SCTP_EVENT_SUBSCRIBE)
    CHECK_STRUCT_HAS_MEMBER("struct sctp_send_failed_event" ssfe_error ${SCTP_INCLUDE} HAVE_SCTP_SEND_FAILED_EVENT)
ENDIF()

IF (HAVE_SCTP_SEND_FAILED_EVENT)
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DHAVE_SCTP_SEND_FAILED_EVENT")
ENDIF()

IF (USRSCTP_SUPPORT)
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DHAVE_SCTP_SEND_FAILED_EVENT")
ENDIF()

IF (HAVE_SCTP_EVENT_SUBSCRIBE)
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DHAVE_SCTP_EVENT_SUBSCRIBE")
ENDIF()

# COMPILER FLAGS
#################################################
# -Wno-array-bounds is unfortunate, but caused by https://bugs.llvm.org/show_bug.cgi?id=20144
SET(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -std=c11   -pedantic -Wall -Wextra -Werror -Wfloat-equal -Wshadow -Wpointer-arith -Wunreachable-code -Winit-self -Wno-unused-function -Wno-unused-parameter -Wno-overlength-strings -Wno-unused-command-line-argument -Wno-unreachable-code -Wno-parentheses-equality -Wno-array-bounds")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pedantic -Wall -Wextra -Werror -Wfloat-equal -Wshadow -Wpointer-arith -Wunreachable-code -Winit-self -Wno-unused-function -Wno-unused-parameter -Wno-overlength-strings -Wno-unused-command-line-argument -Wno-unreachable-code -Wno-parentheses-equality")

IF (CMAKE_BUILD_TYPE MATCHES DEBUG)
    MESSAGE(STATUS "Building in DEBUG mode")
    # _FORTIFY_SOURCE requires compiling with optimization
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O")
ENDIF()

IF (SANITIZER_ADDRESS)
    SET(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -fsanitize=address -O1 -fno-omit-frame-pointer -g -Wno-address-of-packed-member")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -O1 -fno-omit-frame-pointer -g -Wno-address-of-packed-member")
ENDIF()

IF (SANITIZER_MEMORY)
    # maybe add "-fPIE -pie" here
    SET(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -fsanitize=memory -fno-omit-frame-pointer -g -Wno-address-of-packed-member -fsanitize-memory-track-origins")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=memory -fno-omit-frame-pointer -g -Wno-address-of-packed-member -fsanitize-memory-track-origins")
ENDIF()

# LIBRARIES
#################################################
include_directories(.)

LIST(APPEND neat_libs
    ${UV_LIB}
    ${LDNS_LIB}
    ${JANSSON_LIB}
    ${OPENSSL_SSL_LIBRARY}
    ${OPENSSL_CRYPTO_LIBRARY}
    )

IF (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    LIST(APPEND neat_libs ${MNL_LIB})
    LIST(APPEND neat_libs ${SCTP_LIB})
ENDIF()

ADD_LIBRARY(neat SHARED ${neat_sources})

set_target_properties(neat PROPERTIES
    VERSION ${BUILD_VERSION}
    SOVERSION ${BUILD_MAJOR}
    )

ADD_LIBRARY(neat-static STATIC ${neat_sources})

TARGET_LINK_LIBRARIES (neat ${neat_libs})

IF (SWIG_PYTHON)
    SWIG_ADD_MODULE(neat python neat_swig.i ${neat_sources})
    SWIG_LINK_LIBRARIES(neat ${neat_libs} ${PYTHON_LIBRARIES})
ENDIF()

# INSTALL
#################################################
MESSAGE("Install directory: ${CMAKE_INSTALL_PREFIX}")

INSTALL(TARGETS neat neat-static DESTINATION ${CMAKE_INSTALL_LIBDIR})
INSTALL(FILES "neat.h" DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})


# INCLUDE EXAMPLES AND TESTS FOLDER
#################################################
IF (SOCKET_API)
    ADD_SUBDIRECTORY(socketapi)
ENDIF()
ADD_SUBDIRECTORY(examples)
ADD_SUBDIRECTORY(tests)
