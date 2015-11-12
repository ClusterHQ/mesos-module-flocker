add_library(isolator SHARED
        ${CMAKE_CURRENT_SOURCE_DIR}/flocker_isolator.cpp
        ${3rdparty_hdrs}
        ${3rdparty_srcs}
        )

target_link_libraries(isolator
        ${Mesos_LIBRARIES}
        )
