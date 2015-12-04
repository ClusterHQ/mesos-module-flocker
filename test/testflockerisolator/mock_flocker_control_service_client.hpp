#include "gmock/gmock.h"

#include "../../libisolator/flocker_control_service_client.hpp"

#include <stout/try.hpp>

class MockFlockerControlServiceClient : public FlockerControlServiceClient
{
public:

    MockFlockerControlServiceClient(const string ip, uint16_t port, IpUtils *ipUtils) : FlockerControlServiceClient(ip,port,ipUtils) {

    }

    ~MockFlockerControlServiceClient() {}

    MOCK_METHOD0(getNodeId, Try<string>());
    MOCK_METHOD1(createDataSet, Try<string>(UUID uuid));
};