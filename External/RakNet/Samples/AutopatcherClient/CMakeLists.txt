cmake_minimum_required(VERSION 2.6)
project(AutopatcherClient)
IF(WIN32 AND NOT UNIX)
	include_directories(${RAKNETHEADERFILES} ./ ${Autopatcher_SOURCE_DIR} ${BZip2_SOURCE_DIR})
	FILE(GLOB AUTOSRC "${Autopatcher_SOURCE_DIR}/*.cpp" "${Autopatcher_SOURCE_DIR}/*.h")
	FILE(GLOB BZSRC "${BZip2_SOURCE_DIR}/*.c" "${BZip2_SOURCE_DIR}/*.h")
	LIST(REMOVE_ITEM BZSRC "${BZip2_SOURCE_DIR}/dlltest.c" "${BZip2_SOURCE_DIR}/mk251.c" "${BZip2_SOURCE_DIR}/bzip2recover.c")
	SOURCE_GROUP(BZip2 FILES ${BZSRC})
	SET(WRAPFILES "${Autopatcher_SOURCE_DIR}/MemoryCompressor.cpp" "${Autopatcher_SOURCE_DIR}/MemoryCompressor.h")
	LIST(REMOVE_ITEM AUTOSRC ${WRAPFILES})
	SOURCE_GROUP(Client_Files FILES ${AUTOSRC})
	SOURCE_GROUP(MAIN FILES "AutopatcherClientTest.cpp")
	SOURCE_GROUP(BZip2Wrapper FILES ${WRAPFILES})
	add_executable(AutopatcherClient "AutopatcherClientTest.cpp" ${AUTOSRC} ${BZSRC} ${WRAPFILES} "readme.txt")
	target_link_libraries(AutopatcherClient ${RAKNET_COMMON_LIBS})
	VSUBFOLDER(AutopatcherClient "Samples/AutoPatcher/Client")
ELSE(WIN32 AND NOT UNIX)
	include_directories(${RAKNETHEADERFILES} ./ ${Autopatcher_SOURCE_DIR}) 
	add_executable(AutopatcherClient "AutopatcherClientTest.cpp")
	target_link_libraries(AutopatcherClient ${RAKNET_COMMON_LIBS} LibAutopatcher)
ENDIF(WIN32 AND NOT UNIX)






