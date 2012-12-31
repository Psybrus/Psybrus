# CMake generated Testfile for 
# Source directory: C:/Dev/Psybrus/External/pcre
# Build directory: C:/Dev/Psybrus/External/pcre
# 
# This file includes the relevent testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
IF("${CTEST_CONFIGURATION_TYPE}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
  ADD_TEST(pcre_test_bat "pcre_test.bat")
ELSEIF("${CTEST_CONFIGURATION_TYPE}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
  ADD_TEST(pcre_test_bat "pcre_test.bat")
ELSEIF("${CTEST_CONFIGURATION_TYPE}" MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
  ADD_TEST(pcre_test_bat "pcre_test.bat")
ELSEIF("${CTEST_CONFIGURATION_TYPE}" MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
  ADD_TEST(pcre_test_bat "pcre_test.bat")
ELSE()
  ADD_TEST(pcre_test_bat NOT_AVAILABLE)
ENDIF()
SET_TESTS_PROPERTIES(pcre_test_bat PROPERTIES  PASS_REGULAR_EXPRESSION "RunTest\\.bat tests successfully completed")
ADD_TEST(pcrecpp_test "pcrecpp_unittest")
ADD_TEST(pcre_scanner_test "pcre_scanner_unittest")
ADD_TEST(pcre_stringpiece_test "pcre_stringpiece_unittest")
