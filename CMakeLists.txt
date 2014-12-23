# -G "Visual Studio 9 2008"
# First: cd in bulids/type/ (eg: cd builds/arm3730/
# Then, use the below command
#build with arm3730:  cmake ../.. -DCMAKE_TOOLCHAIN_FILE=../../arm_make.cmake -DCMAKE_BUILD_TYPE=Debug
#build with vs2013 :  cmake ../.. -DCMAKE_BUILD_TYPE=Debug -G "Visual Studio 12 2013"
#build with vs2008 :  cmake ../.. -DCMAKE_BUILD_TYPE=Debug -G "Visual Studio 9 2008"
#bjam --toolset=msvc-9.0 --build-type=complete link=static threading=multi runtime-link=static --with-thread

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(vscp)
set_property(GLOBAL PROPERTY USE_FOLDERS On)

SET(PROJECT_SET_NAME_TEST "test")
SET(PROJECT_SET_NAME_LIB "lib")
SET(PROJECT_SET_NAME_RUN "vscp")


if( NOT CMAKE_BUILD_TYPE )
  set( CMAKE_BUILD_TYPE Debug CACHE STRING
       "Setting the default build type with Debug"
       FORCE )
endif()
##########################################################
## Step 1 
if(WIN32)
	SET(VZ_S_BASE_DIR $ENV{VZ_SBASE_PATH})
	if(MSVC90)
		SET(BOOST_1_53_DIR $ENV{BOOST_1_53_0_PATH})
# With VS2013 only support by boost 1.56
	elseif(MSVC12)
		SET(BOOST_1_56_DIR $ENV{BOOST_1_56_0_PATH})
	endif()
elseif(UNIX)
	SET(VZ_S_BASE_DIR "/mnt/hgfs/vz_s_base")
	SET(BOOST_1_53_DIR "/mnt/hgfs/boost_1_53_0")
endif()

##1. INCLUDE_DIRECTORES
if(WIN32)
	if(MSVC90)
	SET(LIBVZNET_INCLUDE_DIR
		${VZ_S_BASE_DIR}/vz_log/glog-0.3.3/src/windows
		${VZ_S_BASE_DIR}/zeromq/libzmq/include
		${BOOST_1_53_DIR}
		# Add Json DIR
		${VZ_S_BASE_DIR}/jsoncpp/include
		${PROJECT_SOURCE_DIR}/src
		)
	elseif(MSVC12)
		SET(LIBVZNET_INCLUDE_DIR
		${VZ_S_BASE_DIR}/vz_log/glog-0.3.3/src/windows
		${VZ_S_BASE_DIR}/zeromq/libzmq/include
		${BOOST_1_56_DIR}
		# Add Json DIR
		${VZ_S_BASE_DIR}/jsoncpp/include
		${PROJECT_SOURCE_DIR}/src
		)
	endif()
elseif(UNIX)
	SET(LIBVZNET_INCLUDE_DIR
	${VZ_S_BASE_DIR}/vz_log/glog-0.3.3/src
	${VZ_S_BASE_DIR}/zeromq/libzmq/include
	${BOOST_1_53_DIR}
	# Add Json DIR
	${VZ_S_BASE_DIR}/jsoncpp/include
	${PROJECT_SOURCE_DIR}/src
	)
endif()

MESSAGE(STATUS "       Add library path and name")
if(CMAKE_BUILD_TYPE MATCHES Debug)
	MESSAGE(STATUS "       Debug Mode")
	if(WIN32)
		if(MSVC90)
			MESSAGE(STATUS "       MSVC90")
			SET(LIBVZNET_LIBRARY_DIR
				${VZ_S_BASE_DIR}/vz_log/glog-0.3.3/lib/vs2008/
				${VZ_S_BASE_DIR}/zeromq/libzmq/lib/vs2008
				${BOOST_1_53_DIR}/stage/lib
				# Add Json library
				${VZ_S_BASE_DIR}/jsoncpp/lib/vs2008
				${PROJECT_SOURCE_DIR}/lib/vs2008
				)
			SET(LIBVZNET_LINK_LIB
				libglog_staticd.lib
				# Add Json library
				json_vc71_libmtd.lib
				libzmqd.lib
				ws2_32.lib
				)
		elseif(MSVC12)
			MESSAGE(STATUS "       MSVC12")
			SET(LIBVZNET_LIBRARY_DIR
				${VZ_S_BASE_DIR}/vz_log/glog-0.3.3/lib/vs2013/
				${VZ_S_BASE_DIR}/zeromq/libzmq/lib/vs2013
				${BOOST_1_56_DIR}/stage/lib
				# Add Json library
				${VZ_S_BASE_DIR}/jsoncpp/lib/vs2013
				${PROJECT_SOURCE_DIR}/lib/vs2013
				)
			SET(LIBVZNET_LINK_LIB
				libglog_staticd.lib
				lib_jsond.lib
				libzmq_d.lib
				ws2_32.lib
				)
		endif()
	elseif(UNIX)
		MESSAGE(STATUS "       UNIX")
		SET(LIBVZNET_LIBRARY_DIR
			${VZ_S_BASE_DIR}/vz_log/glog-0.3.3/lib/arm3730
			${VZ_S_BASE_DIR}/zeromq/libzmq/lib/arm3730
			${BOOST_1_53_DIR}/stage/lib
			# Add Json library
			${VZ_S_BASE_DIR}/jsoncpp/lib/arm3730
			${PROJECT_SOURCE_DIR}/lib/arm3730
			)
		SET(LIBVZNET_LINK_LIB
			libglog.a
			# Add Json library
			libboost_thread.a
			libboost_system.a
			pthread
			rt
			zmq.a
			libjsoncpp.a
			)
	endif()
elseif(CMAKE_BUILD_TYPE MATCHES Release)
	MESSAGE(STATUS "       Release Mode")
	if(WIN32)
		if(MSVC90)
			MESSAGE(STATUS "       MSVC90")
			SET(LIBVZNET_LIBRARY_DIR
				${VZ_S_BASE_DIR}/glog-0.3.3/lib/vs2008/
				${VZ_S_BASE_DIR}/zeromq/libzmq/lib/vs2008
				${BOOST_1_53_DIR}/stage/lib
				# Add Json library
				${VZ_S_BASE_DIR}/jsoncpp/lib/vs2008
				${PROJECT_SOURCE_DIR}/lib/vs2008
				)
			SET(LIBVZNET_LINK_LIB
				libglog_static.lib
				# Add Json library
				json_vc71_libmt.lib
				libzmq.lib
				ws2_32.lib
				)
		elseif(MSVC12)
			MESSAGE(STATUS "       MSVC12")
			SET(LIBVZNET_LIBRARY_DIR
				${VZ_S_BASE_DIR}/glog-0.3.3/lib/vs2013/
				${VZ_S_BASE_DIR}/zeromq/libzmq/lib/vs2013
				${BOOST_1_56_DIR}/stage/lib
				# Add Json library
				${VZ_S_BASE_DIR}/jsoncpp/lib/vs2013
				${PROJECT_SOURCE_DIR}/lib/vs2013
				)
			SET(LIBVZNET_LINK_LIB
				libglog_static.lib
				lib_json.lib
				libzmq.lib
				ws2_32.lib
				)
		endif()
	elseif(UNIX)
		MESSAGE(STATUS "       UNIX")
		if(UNIX_TYPE MATCHES ARM3730)
			SET(LIBVZNET_LIBRARY_DIR
				${VZ_S_BASE_DIR}/vz_log/glog-0.3.3/lib/arm3730
				${VZ_S_BASE_DIR}/zeromq/libzmq/lib/arm3730
				${BOOST_1_53_DIR}/stage/lib
				# Add Json library
				${VZ_S_BASE_DIR}/jsoncpp/lib/arm3730
				${PROJECT_SOURCE_DIR}/lib/arm3730
				)
		ELSEIF(UNIX_TYPE MATCHES ARM6446)
			SET(LIBVZNET_LIBRARY_DIR
				${VZ_S_BASE_DIR}/vz_log/glog-0.3.3/lib/arm6446
				${VZ_S_BASE_DIR}/zeromq/libzmq/lib/arm6446
				${BOOST_1_53_DIR}/arm6446/lib
				# Add Json library
				${VZ_S_BASE_DIR}/jsoncpp/lib/arm6446
				${PROJECT_SOURCE_DIR}/lib/arm6446
				)
		ENDIF()
		SET(LIBVZNET_LINK_LIB
			libglog.a
			# Add Json library
			libboost_thread.a
			libboost_system.a
			pthread
			rt
			#zmq.a
			#libjsoncpp.a
			)
	endif()
endif()

MESSAGE(STATUS "Step 4 : Add code source")
if(UNIX)
	ADD_DEFINITIONS(-DGOOGLE_GLOG_DLL_DECL=
		-DZMQ_STATIC)
elseif(WIN32)
	ADD_DEFINITIONS(-DGOOGLE_GLOG_DLL_DECL=
	-DGLOG_NO_ABBREVIATED_SEVERITIES
	-D_CRT_SECURE_NO_WARNINGS
	-D_WINSOCK_DEPRECATED_NO_WARNINGS
	-D_WIN32_WINNT=0x0502
	-D_SCL_SECURE_NO_WARNINGS
	-DZMQ_STATIC
	)
endif()

INCLUDE_DIRECTORIES(${LIBVZNET_INCLUDE_DIR})
LINK_DIRECTORIES(${LIBVZNET_LIBRARY_DIR})
#####################################################################
# Step 3 :Set visual studio runtime type
if(WIN32)
	set(CompilerFlags
			CMAKE_CXX_FLAGS
			CMAKE_CXX_FLAGS_DEBUG
			CMAKE_CXX_FLAGS_RELEASE
			CMAKE_C_FLAGS
			CMAKE_C_FLAGS_DEBUG
			CMAKE_C_FLAGS_RELEASE
			)
	foreach(CompilerFlag ${CompilerFlags})
	  string(REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
	  string(REPLACE "/MDd" "/MTd" ${CompilerFlag} "${${CompilerFlag}}")
	  #string(REPLACE "/EDITANDCONTINUE" "/SAFESEH" ${CompilerFlag} "${${CompilerFlag}}")
	endforeach()
	
endif(WIN32)

################################################################
if(WIN32)
	if(MSVC90)
		SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY  "${PROJECT_SOURCE_DIR}/bin/vs2008")
		SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY  "${PROJECT_SOURCE_DIR}/lib/vs2008")
		SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY  "${PROJECT_SOURCE_DIR}/lib/vs2008")

		SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG  "${PROJECT_SOURCE_DIR}/bin/vs2008")
		SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG  "${PROJECT_SOURCE_DIR}/lib/vs2008")
		SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG  "${PROJECT_SOURCE_DIR}/lib/vs2008")
	 
		# With Release properties
		SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE  "${PROJECT_SOURCE_DIR}/bin/vs2008")
		SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE  "${PROJECT_SOURCE_DIR}/lib/vs2008")
		SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE  "${PROJECT_SOURCE_DIR}/lib/vs2008")
	elseif(MSVC12)
		SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY  "${PROJECT_SOURCE_DIR}/bin/vs2013")
		SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY  "${PROJECT_SOURCE_DIR}/lib/vs2013")
		SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY  "${PROJECT_SOURCE_DIR}/lib/vs2013")

		SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG  "${PROJECT_SOURCE_DIR}/bin/vs2013")
		SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG  "${PROJECT_SOURCE_DIR}/lib/vs2013")
		SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG  "${PROJECT_SOURCE_DIR}/lib/vs2013")
	 
		# With Release properties
		SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE  "${PROJECT_SOURCE_DIR}/bin/vs2013")
		SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE  "${PROJECT_SOURCE_DIR}/lib/vs2013")
		SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE  "${PROJECT_SOURCE_DIR}/lib/vs2013")
	ENDIF()
elseif(UNIX)
	IF(UNIX_TYPE MATCHES ARM3730)
		SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY  "${PROJECT_SOURCE_DIR}/bin/arm3730")
		SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY  "${PROJECT_SOURCE_DIR}/lib/arm3730")
		SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY  "${PROJECT_SOURCE_DIR}/lib/arm3730")

		SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG  "${PROJECT_SOURCE_DIR}/bin/arm3730")
		SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG  "${PROJECT_SOURCE_DIR}/lib/arm3730")
		SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG  "${PROJECT_SOURCE_DIR}/lib/arm3730")
	 
		# With Release properties
		SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE  "${PROJECT_SOURCE_DIR}/bin/arm6446")
		SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE  "${PROJECT_SOURCE_DIR}/lib/arm6446")
		SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE  "${PROJECT_SOURCE_DIR}/lib/arm6446")
	ELSEIF(UNIX_TYPE MATCHES ARM6446)
		SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY  "${PROJECT_SOURCE_DIR}/bin/arm6446")
		SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY  "${PROJECT_SOURCE_DIR}/lib/arm6446")
		SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY  "${PROJECT_SOURCE_DIR}/lib/arm6446")

		SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG  "${PROJECT_SOURCE_DIR}/bin/arm6446")
		SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG  "${PROJECT_SOURCE_DIR}/lib/arm6446")
		SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG  "${PROJECT_SOURCE_DIR}/lib/arm6446")
	 
		# With Release properties
		SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE  "${PROJECT_SOURCE_DIR}/bin/arm6446")
		SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE  "${PROJECT_SOURCE_DIR}/lib/arm6446")
		SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE  "${PROJECT_SOURCE_DIR}/lib/arm6446")
	ENDIF()
ENDIF()
##########################################################
## Setp 3
if(WIN32)
	#ADD_SUBDIRECTORY(src/tmq)
	#ADD_SUBDIRECTORY(src/test/message_thread_test)
	#ADD_SUBDIRECTORY(src/test/task_test)
endif()

ADD_SUBDIRECTORY(src/vscp)
#ADD_SUBDIRECTORY(src/vzipc)
#
#ADD_SUBDIRECTORY(src/run/vscp_client)
#ADD_SUBDIRECTORY(src/run/vscp_server)
#ADD_SUBDIRECTORY(src/run/vscp_process)
#
#ADD_SUBDIRECTORY(src/test/test_vscp_server)
#ADD_SUBDIRECTORY(src/test/test_vscp_client)
#ADD_SUBDIRECTORY(src/test/test_proto_client)
#ADD_SUBDIRECTORY(src/test/test_proto_server)
#ADD_SUBDIRECTORY(src/test/test_zmq_ipc_client)
#ADD_SUBDIRECTORY(src/test/test_zmq_ipc_server)
#ADD_SUBDIRECTORY(src/test/test_vzipc_server)
#ADD_SUBDIRECTORY(src/test/test_vzipc_client)
ADD_SUBDIRECTORY(src/test/test_pkt_server)
ADD_SUBDIRECTORY(src/test/test_pkt_client)