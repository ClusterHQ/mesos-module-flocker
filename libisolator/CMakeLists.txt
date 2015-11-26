add_library(isolator SHARED
        flocker_isolator.cpp
        flocker-isolator.hpp
        ${Mesos_BUILD_DIR}/src/slave/isolator.pb.cc
        )

target_link_libraries(isolator
        ${Mesos_LIBRARIES}
        )

set(Boost_Version 1.49.0)
string(REPLACE "." "_" Boost_Version_Underscore ${Boost_Version})
set(Boost_DOWNLOAD_PATH /tmp/boost_${Boost_Version_Underscore}.zip)
set(Boost_URL http://downloads.sourceforge.net/project/boost/boost/${Boost_Version}/boost_${Boost_Version_Underscore}.zip)
set(Boost_EXTRACTED_FILE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

if (NOT EXISTS "${Boost_DOWNLOAD_PATH}")
    message(STATUS "Downloading Boost ${Boost_Version} to /tmp")
    file(DOWNLOAD "${Boost_URL}" "${Boost_DOWNLOAD_PATH}")
    message(STATUS "Unzipping Boost ${Boost_Version} to ${Boost_EXTRACTED_FILE}")
    execute_process(COMMAND unzip -qo ${Boost_DOWNLOAD_PATH} -d ${Boost_EXTRACTED_FILE})
endif()

include_directories(${Boost_EXTRACTED_FILE}/boost_${Boost_Version_Underscore})
