cmake_minimum_required(VERSION 2.8)

project(QuteCalibrator)


find_package(X11 REQUIRED)

message(STATUS "X11_FOUND = ${X11_FOUND}")
message(STATUS "X11_INCLUDE_DIR = ${X11_INCLUDE_DIR}")
message(STATUS "X11_LIBRARIES = ${X11_LIBRARIES}")

include_directories(${X11_INCLUDE_DIR})

link_directories(${X11_LIBRARIES})


find_package(PkgConfig)

pkg_check_modules(GTKMM gtkmm-2.4) # look into FindPkgConfig.cmake

pkg_check_modules(GTK2 gtk+-2.0) # look into FindPkgConfig.cmake

link_directories(${GTKMM_LIBRARY_DIRS})

include_directories(${GTKMM_INCLUDE_DIRS})

include_directories (${GTK2_INCLUDE_DIRS})

link_directories (${GTK2_LIBRARY_DIRS})

# configure a header file to pass some of the CMake settings
# to the source code
#configure_file (
#  "${PROJECT_SOURCE_DIR}"
#  "${PROJECT_BINARY_DIR}"
#  )
 
# add the binary tree to the search path for include files
# so that we will find TutorialConfig.h
include_directories("${PROJECT_SOURCE_DIR}")
include_directories("${PROJECT_SOURCE_DIR}/gui")
include_directories("${PROJECT_SOURCE_DIR}/calibrator")


set(TARGET_EXE ../xinput_calibrator)

#set(CMAKE_CXX_FLAGS "-std=c++11  -Ofast -s")
set(CMAKE_CXX_FLAGS "-std=c++11  -g")

set(HEADER calibrator.hh 
           calibratorbuilder.hh 
           lang.hh
           calibrator/Evdev.hpp 
           calibrator/EvdevTester.hpp 
           calibrator/Tester.hpp 
           calibrator/Usbtouchscreen.hpp 
           calibrator/XorgPrint.hpp            
           rapidjson/allocators.h
           rapidjson/cursorstreamwrapper.h
           rapidjson/document.h
           rapidjson/encodedstream.h
           rapidjson/encodings.h
           rapidjson/filereadstream.h
           rapidjson/filewritestream.h
           rapidjson/fwd.h
           rapidjson/istreamwrapper.h
           rapidjson/memorybuffer.h
           rapidjson/memorystream.h
           rapidjson/ostreamwrapper.h
           rapidjson/pointer.h
           rapidjson/prettywriter.h
           rapidjson/rapidjson.h
           rapidjson/reader.h
           rapidjson/schema.h
           rapidjson/stream.h
           rapidjson/stringbuffer.h
           rapidjson/writer.h
           rapidjson/error/en.h
           rapidjson/error/error.h
           rapidjson/internal/biginteger.h
           rapidjson/internal/diyfp.h
           rapidjson/internal/dtoa.h
           rapidjson/internal/ieee754.h
           rapidjson/internal/itoa.h
           rapidjson/internal/meta.h
           rapidjson/internal/pow10.h
           rapidjson/internal/regex.h
           rapidjson/internal/stack.h
           rapidjson/internal/strfunc.h
           rapidjson/internal/strtod.h
           rapidjson/internal/swap.h
           rapidjson/msinttypes/inttypes.h
           rapidjson/msinttypes/stdint.h
           gui/gtkmm.hpp 
           gui/gui_common.hpp)


set(SOURCE calibrator.cpp
           calibratorbuilder.cpp
           lang.cpp
           main.cpp
           main_common.cpp
           calibrator/Evdev.cpp
           calibrator/EvdevTester.cpp
           calibrator/Tester.cpp
           calibrator/Usbtouchscreen.cpp
           calibrator/XorgPrint.cpp
           gui/gtkmm.cpp
           gui/gui_common.cpp)


add_executable(${TARGET_EXE} ${HEADER} ${SOURCE})

target_link_libraries(${TARGET_EXE}
	${X11_LIBRARIES} 
	${GTKMM_LIBRARY_DIRS}
        -lX11
	-lXi
	-lgtkmm-2.4 
	-latkmm-1.6 
	-lgdkmm-2.4 
	-lgiomm-2.4 
	-lpangomm-1.4 
	-lglibmm-2.4 
	-lcairomm-1.0 
	-lsigc-2.0 
	-lgtk-x11-2.0 
	-lgdk-x11-2.0 
	-lpangocairo-1.0 
	-latk-1.0 
	-lcairo 
	-lgdk_pixbuf-2.0 
	-lgio-2.0 
	-lpangoft2-1.0 
	-lpango-1.0 
	-lgobject-2.0 
	-lglib-2.0 
	-lfontconfig
	)
	
