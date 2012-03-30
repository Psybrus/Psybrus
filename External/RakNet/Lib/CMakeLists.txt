cmake_minimum_required(VERSION 2.6)

IF(NOT DISABLE_STATICLIB)
	add_subdirectory(LibStatic)
ENDIF(NOT DISABLE_STATICLIB)

IF(NOT DISABLE_DLL)
	add_subdirectory(DLL)
ENDIF(NOT DISABLE_DLL)