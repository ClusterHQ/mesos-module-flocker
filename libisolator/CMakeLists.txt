add_library(isolator SHARED
        flocker_isolator.cpp
        flocker-isolator.hpp
        ${Mesos_BUILD_DIR}/src/slave/isolator.pb.cc
        flocker_control_service_client.cpp
        flocker_control_service_client.hpp)

target_link_libraries(isolator
        ${Mesos_LIBRARIES}
        )
