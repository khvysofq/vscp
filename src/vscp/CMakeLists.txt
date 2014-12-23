cmake_minimum_required(VERSION 2.8)
#########################################################################

# Basic environment setting

if(WIN32)
	SET(BUILD_PROJECT_NAME "libvscp")
elseif(UNIX)
	SET(BUILD_PROJECT_NAME "vscp")
endif()

#########################################################################
#INCLUDE_DIRECTORIES(${LIBVZNET_INCLUDE_DIR})
MESSAGE(STATUS "Print the include directores")
get_property(inc_dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
foreach(inc_dir ${inc_dirs})
  message(STATUS "    include directory='${inc_dir}'")
endforeach()

#########################################################################
#Step 2 : Add library directories
MESSAGE(STATUS "Step 2 : Add library directories")
#LINK_DIRECTORIES(${LIBVZNET_LIBRARY_DIR})
MESSAGE(STATUS "Print the link directores")
get_property(link_dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY LINK_DIRECTORIES)
foreach(link_dir ${link_dirs})
  message(STATUS "    link directory='${link_dir}'")
endforeach()

#########################################################################
#Step 3 : Add code source
SET(SRC_LIST 
	${CMAKE_CURRENT_SOURCE_DIR}/net/basesession.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/net/basesession.h
	${CMAKE_CURRENT_SOURCE_DIR}/net/basesessionmanager.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/net/basesessionmanager.h
	${CMAKE_CURRENT_SOURCE_DIR}/net/vscpserver.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/net/vscpserver.h
	${CMAKE_CURRENT_SOURCE_DIR}/net/packettransmitter.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/net/packettransmitter.h
	${CMAKE_CURRENT_SOURCE_DIR}/net/writehandler.h
	${CMAKE_CURRENT_SOURCE_DIR}/net/vscpservices.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/net/vscpservices.h
	
	${CMAKE_CURRENT_SOURCE_DIR}/base/basictypes.h
	${CMAKE_CURRENT_SOURCE_DIR}/base/basicdefines.h
	${CMAKE_CURRENT_SOURCE_DIR}/base/helper.h
	${CMAKE_CURRENT_SOURCE_DIR}/base/helper.cpp
	
	#${CMAKE_CURRENT_SOURCE_DIR}/proto/protobasicdefines.h
	#${CMAKE_CURRENT_SOURCE_DIR}/proto/protobasicincludes.h
	#${CMAKE_CURRENT_SOURCE_DIR}/proto/baseprotoanalyzer.cpp
	#${CMAKE_CURRENT_SOURCE_DIR}/proto/baseprotoanalyzer.h
	#${CMAKE_CURRENT_SOURCE_DIR}/proto/serverprotoanalyzer.h
	#${CMAKE_CURRENT_SOURCE_DIR}/proto/serverprotoanalyzer.cpp
	#${CMAKE_CURRENT_SOURCE_DIR}/proto/clientprotoanalyzer.h
	#${CMAKE_CURRENT_SOURCE_DIR}/proto/clientprotoanalyzer.cpp
	#${CMAKE_CURRENT_SOURCE_DIR}/proto/vsid.cpp
	#${CMAKE_CURRENT_SOURCE_DIR}/proto/vsid.h
	#
	#${CMAKE_CURRENT_SOURCE_DIR}/vscp/vscpbasesession.h
	#${CMAKE_CURRENT_SOURCE_DIR}/vscp/vscpbasesession.cpp
	#${CMAKE_CURRENT_SOURCE_DIR}/vscp/vscpsessionmanager.h
	#${CMAKE_CURRENT_SOURCE_DIR}/vscp/vscpsessionmanager.cpp
	)


SOURCE_GROUP(net FILES
	${CMAKE_CURRENT_SOURCE_DIR}/net/basesession.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/net/basesession.h
	${CMAKE_CURRENT_SOURCE_DIR}/net/basesessionmanager.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/net/basesessionmanager.h
	${CMAKE_CURRENT_SOURCE_DIR}/net/vscpserver.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/net/vscpserver.h
	${CMAKE_CURRENT_SOURCE_DIR}/net/packettransmitter.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/net/packettransmitter.h
	${CMAKE_CURRENT_SOURCE_DIR}/net/writehandler.h
	${CMAKE_CURRENT_SOURCE_DIR}/net/vscpservices.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/net/vscpservices.h
	)

SOURCE_GROUP(base FILES
	${CMAKE_CURRENT_SOURCE_DIR}/base/basictypes.h
	${CMAKE_CURRENT_SOURCE_DIR}/base/basicdefines.h
	${CMAKE_CURRENT_SOURCE_DIR}/base/helper.h
	${CMAKE_CURRENT_SOURCE_DIR}/base/helper.cpp
	)

#SOURCE_GROUP(proto FILES
#	${CMAKE_CURRENT_SOURCE_DIR}/proto/protobasicdefines.h
#	${CMAKE_CURRENT_SOURCE_DIR}/proto/protobasicincludes.h
#	${CMAKE_CURRENT_SOURCE_DIR}/proto/baseprotoanalyzer.cpp
#	${CMAKE_CURRENT_SOURCE_DIR}/proto/baseprotoanalyzer.h
#	${CMAKE_CURRENT_SOURCE_DIR}/proto/serverprotoanalyzer.h
#	${CMAKE_CURRENT_SOURCE_DIR}/proto/serverprotoanalyzer.cpp
#	${CMAKE_CURRENT_SOURCE_DIR}/proto/clientprotoanalyzer.h
#	${CMAKE_CURRENT_SOURCE_DIR}/proto/clientprotoanalyzer.cpp
#	${CMAKE_CURRENT_SOURCE_DIR}/proto/vsid.cpp
#	${CMAKE_CURRENT_SOURCE_DIR}/proto/vsid.h
#	)
#
#SOURCE_GROUP(vscp FILES
#	${CMAKE_CURRENT_SOURCE_DIR}/vscp/vscpbasesession.h
#	${CMAKE_CURRENT_SOURCE_DIR}/vscp/vscpbasesession.cpp
#	${CMAKE_CURRENT_SOURCE_DIR}/vscp/vscpsessionmanager.h
#	${CMAKE_CURRENT_SOURCE_DIR}/vscp/vscpsessionmanager.cpp
#	)
#########################################################################
#Step 4 : Add PROJECT define 
MESSAGE(STATUS "Step 4 : Add code source")

#########################################################################
#Step 5 : Add executable or library target
MESSAGE(STATUS "Step 5 : Add executable or library target")
#ADD_executable(${BUILD_PROJECT_NAME} ${SRC_LIST})
ADD_LIBRARY(${BUILD_PROJECT_NAME} STATIC ${SRC_LIST})
set_property(TARGET ${BUILD_PROJECT_NAME} PROPERTY FOLDER ${PROJECT_SET_NAME_LIB})
#########################################################################
#Step 6 : link with other library
MESSAGE(STATUS "Step 6 : link with other library")

TARGET_LINK_LIBRARIES(${BUILD_PROJECT_NAME} 
	${LIBVZNET_LINK_LIB}
)