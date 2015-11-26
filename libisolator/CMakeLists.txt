add_library(isolator SHARED
        flocker_isolator.cpp
        flocker-isolator.hpp
        ${Mesos_BUILD_DIR}/src/slave/isolator.pb.cc
        )

target_link_libraries(isolator
        ${Mesos_LIBRARIES}
        )
