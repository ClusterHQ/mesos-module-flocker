file(GLOB SRCS *.cpp)

ADD_EXECUTABLE(test_flocker_isolator ${SRCS})

TARGET_LINK_LIBRARIES(test_flocker_isolator
        isolator
        ${MESOS_LIBRARIES}
        glog
        protobuf
        libgtest
        libgmock
        )

add_test(NAME test_flocker_isolator
        COMMAND test_flocker_isolator)
