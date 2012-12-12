# Install script for directory: C:/Dev/Psybrus/External/pcre

# Set the install prefix
IF(NOT DEFINED CMAKE_INSTALL_PREFIX)
  SET(CMAKE_INSTALL_PREFIX "C:/Program Files (x86)/PCRE")
ENDIF(NOT DEFINED CMAKE_INSTALL_PREFIX)
STRING(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
IF(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  IF(BUILD_TYPE)
    STRING(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  ELSE(BUILD_TYPE)
    SET(CMAKE_INSTALL_CONFIG_NAME "Release")
  ENDIF(BUILD_TYPE)
  MESSAGE(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
ENDIF(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)

# Set the component getting installed.
IF(NOT CMAKE_INSTALL_COMPONENT)
  IF(COMPONENT)
    MESSAGE(STATUS "Install component: \"${COMPONENT}\"")
    SET(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  ELSE(COMPONENT)
    SET(CMAKE_INSTALL_COMPONENT)
  ENDIF(COMPONENT)
ENDIF(NOT CMAKE_INSTALL_COMPONENT)

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  IF("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "C:/Dev/Psybrus/External/pcre/Debug/pcred.lib")
  ELSEIF("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "C:/Dev/Psybrus/External/pcre/Release/pcre.lib")
  ELSEIF("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
    FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "C:/Dev/Psybrus/External/pcre/MinSizeRel/pcre.lib")
  ELSEIF("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
    FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "C:/Dev/Psybrus/External/pcre/RelWithDebInfo/pcre.lib")
  ENDIF()
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  IF("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "C:/Dev/Psybrus/External/pcre/Debug/pcreposixd.lib")
  ELSEIF("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "C:/Dev/Psybrus/External/pcre/Release/pcreposix.lib")
  ELSEIF("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
    FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "C:/Dev/Psybrus/External/pcre/MinSizeRel/pcreposix.lib")
  ELSEIF("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
    FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "C:/Dev/Psybrus/External/pcre/RelWithDebInfo/pcreposix.lib")
  ENDIF()
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  IF("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "C:/Dev/Psybrus/External/pcre/Debug/pcrecppd.lib")
  ELSEIF("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "C:/Dev/Psybrus/External/pcre/Release/pcrecpp.lib")
  ELSEIF("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
    FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "C:/Dev/Psybrus/External/pcre/MinSizeRel/pcrecpp.lib")
  ELSEIF("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
    FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "C:/Dev/Psybrus/External/pcre/RelWithDebInfo/pcrecpp.lib")
  ENDIF()
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  IF("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE EXECUTABLE FILES "C:/Dev/Psybrus/External/pcre/DEBUG/pcregrep.exe")
  ELSEIF("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE EXECUTABLE FILES "C:/Dev/Psybrus/External/pcre/Release/pcregrep.exe")
  ELSEIF("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
    FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE EXECUTABLE FILES "C:/Dev/Psybrus/External/pcre/MinSizeRel/pcregrep.exe")
  ELSEIF("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
    FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE EXECUTABLE FILES "C:/Dev/Psybrus/External/pcre/RelWithDebInfo/pcregrep.exe")
  ENDIF()
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  IF("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE EXECUTABLE FILES "C:/Dev/Psybrus/External/pcre/DEBUG/pcretest.exe")
  ELSEIF("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE EXECUTABLE FILES "C:/Dev/Psybrus/External/pcre/Release/pcretest.exe")
  ELSEIF("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
    FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE EXECUTABLE FILES "C:/Dev/Psybrus/External/pcre/MinSizeRel/pcretest.exe")
  ELSEIF("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
    FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE EXECUTABLE FILES "C:/Dev/Psybrus/External/pcre/RelWithDebInfo/pcretest.exe")
  ENDIF()
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  IF("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE EXECUTABLE FILES "C:/Dev/Psybrus/External/pcre/Debug/pcrecpp_unittest.exe")
  ELSEIF("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE EXECUTABLE FILES "C:/Dev/Psybrus/External/pcre/Release/pcrecpp_unittest.exe")
  ELSEIF("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
    FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE EXECUTABLE FILES "C:/Dev/Psybrus/External/pcre/MinSizeRel/pcrecpp_unittest.exe")
  ELSEIF("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
    FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE EXECUTABLE FILES "C:/Dev/Psybrus/External/pcre/RelWithDebInfo/pcrecpp_unittest.exe")
  ENDIF()
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  IF("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE EXECUTABLE FILES "C:/Dev/Psybrus/External/pcre/Debug/pcre_scanner_unittest.exe")
  ELSEIF("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE EXECUTABLE FILES "C:/Dev/Psybrus/External/pcre/Release/pcre_scanner_unittest.exe")
  ELSEIF("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
    FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE EXECUTABLE FILES "C:/Dev/Psybrus/External/pcre/MinSizeRel/pcre_scanner_unittest.exe")
  ELSEIF("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
    FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE EXECUTABLE FILES "C:/Dev/Psybrus/External/pcre/RelWithDebInfo/pcre_scanner_unittest.exe")
  ENDIF()
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  IF("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE EXECUTABLE FILES "C:/Dev/Psybrus/External/pcre/Debug/pcre_stringpiece_unittest.exe")
  ELSEIF("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE EXECUTABLE FILES "C:/Dev/Psybrus/External/pcre/Release/pcre_stringpiece_unittest.exe")
  ELSEIF("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
    FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE EXECUTABLE FILES "C:/Dev/Psybrus/External/pcre/MinSizeRel/pcre_stringpiece_unittest.exe")
  ELSEIF("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
    FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE EXECUTABLE FILES "C:/Dev/Psybrus/External/pcre/RelWithDebInfo/pcre_stringpiece_unittest.exe")
  ENDIF()
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include" TYPE FILE FILES
    "C:/Dev/Psybrus/External/pcre/pcre.h"
    "C:/Dev/Psybrus/External/pcre/pcreposix.h"
    )
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include" TYPE FILE FILES
    "C:/Dev/Psybrus/External/pcre/pcrecpp.h"
    "C:/Dev/Psybrus/External/pcre/pcre_scanner.h"
    "C:/Dev/Psybrus/External/pcre/pcrecpparg.h"
    "C:/Dev/Psybrus/External/pcre/pcre_stringpiece.h"
    )
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/man/man1" TYPE FILE FILES
    "C:/Dev/Psybrus/External/pcre/doc/pcre-config.1"
    "C:/Dev/Psybrus/External/pcre/doc/pcregrep.1"
    "C:/Dev/Psybrus/External/pcre/doc/pcretest.1"
    )
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/man/man3" TYPE FILE FILES
    "C:/Dev/Psybrus/External/pcre/doc/pcre.3"
    "C:/Dev/Psybrus/External/pcre/doc/pcre16.3"
    "C:/Dev/Psybrus/External/pcre/doc/pcre32.3"
    "C:/Dev/Psybrus/External/pcre/doc/pcreapi.3"
    "C:/Dev/Psybrus/External/pcre/doc/pcrebuild.3"
    "C:/Dev/Psybrus/External/pcre/doc/pcrecallout.3"
    "C:/Dev/Psybrus/External/pcre/doc/pcrecompat.3"
    "C:/Dev/Psybrus/External/pcre/doc/pcrecpp.3"
    "C:/Dev/Psybrus/External/pcre/doc/pcredemo.3"
    "C:/Dev/Psybrus/External/pcre/doc/pcrejit.3"
    "C:/Dev/Psybrus/External/pcre/doc/pcrelimits.3"
    "C:/Dev/Psybrus/External/pcre/doc/pcrematching.3"
    "C:/Dev/Psybrus/External/pcre/doc/pcrepartial.3"
    "C:/Dev/Psybrus/External/pcre/doc/pcrepattern.3"
    "C:/Dev/Psybrus/External/pcre/doc/pcreperform.3"
    "C:/Dev/Psybrus/External/pcre/doc/pcreposix.3"
    "C:/Dev/Psybrus/External/pcre/doc/pcreprecompile.3"
    "C:/Dev/Psybrus/External/pcre/doc/pcresample.3"
    "C:/Dev/Psybrus/External/pcre/doc/pcrestack.3"
    "C:/Dev/Psybrus/External/pcre/doc/pcresyntax.3"
    "C:/Dev/Psybrus/External/pcre/doc/pcreunicode.3"
    "C:/Dev/Psybrus/External/pcre/doc/pcre_assign_jit_stack.3"
    "C:/Dev/Psybrus/External/pcre/doc/pcre_compile.3"
    "C:/Dev/Psybrus/External/pcre/doc/pcre_compile2.3"
    "C:/Dev/Psybrus/External/pcre/doc/pcre_config.3"
    "C:/Dev/Psybrus/External/pcre/doc/pcre_copy_named_substring.3"
    "C:/Dev/Psybrus/External/pcre/doc/pcre_copy_substring.3"
    "C:/Dev/Psybrus/External/pcre/doc/pcre_dfa_exec.3"
    "C:/Dev/Psybrus/External/pcre/doc/pcre_exec.3"
    "C:/Dev/Psybrus/External/pcre/doc/pcre_free_study.3"
    "C:/Dev/Psybrus/External/pcre/doc/pcre_free_substring.3"
    "C:/Dev/Psybrus/External/pcre/doc/pcre_free_substring_list.3"
    "C:/Dev/Psybrus/External/pcre/doc/pcre_fullinfo.3"
    "C:/Dev/Psybrus/External/pcre/doc/pcre_get_named_substring.3"
    "C:/Dev/Psybrus/External/pcre/doc/pcre_get_stringnumber.3"
    "C:/Dev/Psybrus/External/pcre/doc/pcre_get_stringtable_entries.3"
    "C:/Dev/Psybrus/External/pcre/doc/pcre_get_substring.3"
    "C:/Dev/Psybrus/External/pcre/doc/pcre_get_substring_list.3"
    "C:/Dev/Psybrus/External/pcre/doc/pcre_jit_exec.3"
    "C:/Dev/Psybrus/External/pcre/doc/pcre_jit_stack_alloc.3"
    "C:/Dev/Psybrus/External/pcre/doc/pcre_jit_stack_free.3"
    "C:/Dev/Psybrus/External/pcre/doc/pcre_maketables.3"
    "C:/Dev/Psybrus/External/pcre/doc/pcre_pattern_to_host_byte_order.3"
    "C:/Dev/Psybrus/External/pcre/doc/pcre_refcount.3"
    "C:/Dev/Psybrus/External/pcre/doc/pcre_study.3"
    "C:/Dev/Psybrus/External/pcre/doc/pcre_utf16_to_host_byte_order.3"
    "C:/Dev/Psybrus/External/pcre/doc/pcre_utf32_to_host_byte_order.3"
    "C:/Dev/Psybrus/External/pcre/doc/pcre_version.3"
    )
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/doc/pcre/html" TYPE FILE FILES
    "C:/Dev/Psybrus/External/pcre/doc/html/index.html"
    "C:/Dev/Psybrus/External/pcre/doc/html/pcre-config.html"
    "C:/Dev/Psybrus/External/pcre/doc/html/pcre.html"
    "C:/Dev/Psybrus/External/pcre/doc/html/pcre16.html"
    "C:/Dev/Psybrus/External/pcre/doc/html/pcre32.html"
    "C:/Dev/Psybrus/External/pcre/doc/html/pcreapi.html"
    "C:/Dev/Psybrus/External/pcre/doc/html/pcrebuild.html"
    "C:/Dev/Psybrus/External/pcre/doc/html/pcrecallout.html"
    "C:/Dev/Psybrus/External/pcre/doc/html/pcrecompat.html"
    "C:/Dev/Psybrus/External/pcre/doc/html/pcrecpp.html"
    "C:/Dev/Psybrus/External/pcre/doc/html/pcredemo.html"
    "C:/Dev/Psybrus/External/pcre/doc/html/pcregrep.html"
    "C:/Dev/Psybrus/External/pcre/doc/html/pcrejit.html"
    "C:/Dev/Psybrus/External/pcre/doc/html/pcrelimits.html"
    "C:/Dev/Psybrus/External/pcre/doc/html/pcrematching.html"
    "C:/Dev/Psybrus/External/pcre/doc/html/pcrepartial.html"
    "C:/Dev/Psybrus/External/pcre/doc/html/pcrepattern.html"
    "C:/Dev/Psybrus/External/pcre/doc/html/pcreperform.html"
    "C:/Dev/Psybrus/External/pcre/doc/html/pcreposix.html"
    "C:/Dev/Psybrus/External/pcre/doc/html/pcreprecompile.html"
    "C:/Dev/Psybrus/External/pcre/doc/html/pcresample.html"
    "C:/Dev/Psybrus/External/pcre/doc/html/pcrestack.html"
    "C:/Dev/Psybrus/External/pcre/doc/html/pcresyntax.html"
    "C:/Dev/Psybrus/External/pcre/doc/html/pcretest.html"
    "C:/Dev/Psybrus/External/pcre/doc/html/pcreunicode.html"
    "C:/Dev/Psybrus/External/pcre/doc/html/pcre_assign_jit_stack.html"
    "C:/Dev/Psybrus/External/pcre/doc/html/pcre_compile.html"
    "C:/Dev/Psybrus/External/pcre/doc/html/pcre_compile2.html"
    "C:/Dev/Psybrus/External/pcre/doc/html/pcre_config.html"
    "C:/Dev/Psybrus/External/pcre/doc/html/pcre_copy_named_substring.html"
    "C:/Dev/Psybrus/External/pcre/doc/html/pcre_copy_substring.html"
    "C:/Dev/Psybrus/External/pcre/doc/html/pcre_dfa_exec.html"
    "C:/Dev/Psybrus/External/pcre/doc/html/pcre_exec.html"
    "C:/Dev/Psybrus/External/pcre/doc/html/pcre_free_study.html"
    "C:/Dev/Psybrus/External/pcre/doc/html/pcre_free_substring.html"
    "C:/Dev/Psybrus/External/pcre/doc/html/pcre_free_substring_list.html"
    "C:/Dev/Psybrus/External/pcre/doc/html/pcre_fullinfo.html"
    "C:/Dev/Psybrus/External/pcre/doc/html/pcre_get_named_substring.html"
    "C:/Dev/Psybrus/External/pcre/doc/html/pcre_get_stringnumber.html"
    "C:/Dev/Psybrus/External/pcre/doc/html/pcre_get_stringtable_entries.html"
    "C:/Dev/Psybrus/External/pcre/doc/html/pcre_get_substring.html"
    "C:/Dev/Psybrus/External/pcre/doc/html/pcre_get_substring_list.html"
    "C:/Dev/Psybrus/External/pcre/doc/html/pcre_jit_exec.html"
    "C:/Dev/Psybrus/External/pcre/doc/html/pcre_jit_stack_alloc.html"
    "C:/Dev/Psybrus/External/pcre/doc/html/pcre_jit_stack_free.html"
    "C:/Dev/Psybrus/External/pcre/doc/html/pcre_maketables.html"
    "C:/Dev/Psybrus/External/pcre/doc/html/pcre_pattern_to_host_byte_order.html"
    "C:/Dev/Psybrus/External/pcre/doc/html/pcre_refcount.html"
    "C:/Dev/Psybrus/External/pcre/doc/html/pcre_study.html"
    "C:/Dev/Psybrus/External/pcre/doc/html/pcre_utf16_to_host_byte_order.html"
    "C:/Dev/Psybrus/External/pcre/doc/html/pcre_utf32_to_host_byte_order.html"
    "C:/Dev/Psybrus/External/pcre/doc/html/pcre_version.html"
    )
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

IF(CMAKE_INSTALL_COMPONENT)
  SET(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
ELSE(CMAKE_INSTALL_COMPONENT)
  SET(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
ENDIF(CMAKE_INSTALL_COMPONENT)

FILE(WRITE "C:/Dev/Psybrus/External/pcre/${CMAKE_INSTALL_MANIFEST}" "")
FOREACH(file ${CMAKE_INSTALL_MANIFEST_FILES})
  FILE(APPEND "C:/Dev/Psybrus/External/pcre/${CMAKE_INSTALL_MANIFEST}" "${file}\n")
ENDFOREACH(file)
