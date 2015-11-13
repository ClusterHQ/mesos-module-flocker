add_library(isolator SHARED
        ${CMAKE_CURRENT_SOURCE_DIR}/flocker_isolator.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/flocker-isolator.hpp
        ${Mesos_BUILD_DIR}/src/slave/isolator.pb.cc
        )

target_link_libraries(isolator
        ${Mesos_LIBRARIES}
        )
