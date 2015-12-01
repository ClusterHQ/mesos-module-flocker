#include <stout/try.hpp>
#include "../../libisolator/flocker-isolator.hpp"
#include "mock_flocker_control_service_client.hpp"
#include <stout/os.hpp>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "test_flocker_isolator.hpp"
#include "../../libisolator/FlockerEnvironmentalVariables.h"

using ::testing::Return;

using namespace mesos::slave;
using namespace mesos;
using namespace std;

FlockerIsolatorTest::FlockerIsolatorTest() { };

FlockerIsolatorTest::~FlockerIsolatorTest() {};

void FlockerIsolatorTest::SetUp() {};

void FlockerIsolatorTest::TearDown() {};

TEST_F(FlockerIsolatorTest, IsolatorCreateSetsFlockerIpAndPort) {

    Parameters parameters;

    string flockerControlIp = "192.1.2.3";
    Parameter* parameter = parameters.add_parameter();
    parameter->set_key("flocker_control_ip");
    parameter->set_value(flockerControlIp);

    int flockerControlPort = 4523;
    parameter = parameters.add_parameter();
    parameter->set_key("flocker_control_port");
    parameter->set_value(stringify(flockerControlPort));

    Try<FlockerIsolator*> result = FlockerIsolator::create(parameters);
    if (result.isError()) {
        cerr << "Could not create Flocker isolator" << endl;
    }

    EXPECT_EQ(flockerControlIp, result.get()->getFlockerControlClient()->getFlockerControlIp());
    EXPECT_EQ(flockerControlPort, result.get()->getFlockerControlClient()->getFlockerControlPort());
}

TEST_F(FlockerIsolatorTest, IsolatorCreateWithoutParametersReturnsError) {
    Parameters parameters;

    Try<FlockerIsolator*> result = FlockerIsolator::create(parameters);

    EXPECT_TRUE(result.isError());
}

TEST_F(FlockerIsolatorTest, IsolatorCreateWithoutIpReturnsError) {
    Parameters parameters;

    int flockerControlPort = 4523;
    Parameter* parameter = parameters.add_parameter();
    parameter->set_key("flocker_control_port");
    parameter->set_value(stringify(flockerControlPort));

    Try<FlockerIsolator*> result = FlockerIsolator::create(parameters);

    EXPECT_TRUE(result.isError());
}

TEST_F(FlockerIsolatorTest, IsolatorCreateWithoutPortReturnsError) {
    Parameters parameters;

    string flockerControlIp = "192.1.2.3";
    Parameter* parameter = parameters.add_parameter();
    parameter->set_key("flocker_control_ip");
    parameter->set_value(flockerControlIp);

    Try<FlockerIsolator*> result = FlockerIsolator::create(parameters);

    EXPECT_TRUE(result.isError());
}

TEST_F(FlockerIsolatorTest, IsolatorPrepareCallsFlockerControlService) {

    const string ip = "192.1.2.3";
    uint16_t port = 1234;

    MockFlockerControlServiceClient flockerControlClient(ip, port);

    EXPECT_CALL(flockerControlClient, getNodeId()).WillOnce(Return(Try<string>::some("fef7fa02-c8c2-4c52-96b5-de70a8ef1925")));

    UUID uuid = UUID::fromString("fef7fa02-c8c2-4c52-96b5-de70a8ef1925");

    EXPECT_CALL(flockerControlClient, createDataSet(uuid)).WillOnce(Return(Try<string>::some(
        "{\"deleted\": false, \"dataset_id\": \"e66d949c-ae91-4446-9115-824722a1e4b0\", \"primary\": \"fef7fa02-c8c2-4c52-96b5-de70a8ef1925\", \"metadata\": {}}"
    )));

    FlockerIsolator *isolator = new FlockerIsolator(&flockerControlClient);

    ContainerID containerId;
    containerId.set_value("befa2b13da05");

    ExecutorInfo executor;
    executor.mutable_executor_id()->set_value("default");
    executor.mutable_command()->set_value("/bin/sleep");
    executor.mutable_command()->add_arguments("60");
    Environment_Variable *envVar = executor.mutable_command()->mutable_environment()->add_variables();
    envVar->set_name(FlockerEnvironmentalVariables::FLOCKER_CONTAINER_VOLUME_PATH);
    envVar->set_value("/data/volume1");
    executor.set_name("Test Executor (/bin/sleep)");

    Result<string> user = os::user();

    isolator->prepare(containerId, executor, "/tmp", user.get());

    EXPECT_EQ(1, 1);
}

// TODO (Frank): Extract parser and move these tests to parser unit test

TEST_F(FlockerIsolatorTest, TestGetFlockerDataSetUUID) {
    std::string json = "{\"deleted\": false, \"dataset_id\": \"e66d949c-ae91-4446-9115-824722a1e4b0\", \"primary\": \"fef7fa02-c8c2-4c52-96b5-de70a8ef1925\", \"metadata\": {}}";

    FlockerControlServiceClient *client = new FlockerControlServiceClient("192.168.1.1", 80);

    const string datasetUUID = client->getFlockerDataSetUUID(json);

    ASSERT_EQ(datasetUUID, "e66d949c-ae91-4446-9115-824722a1e4b0");
}

TEST_F(FlockerIsolatorTest, TestParseNodeId) {
    Try<string> json = Try<string>::some("[{\"host\": \"192.168.1.101\", \"uuid\": \"fef7fa02-c8c2-4c52-96b5-de70a8ef1925\"}, {\"host\": \"10.0.0.200\", \"uuid\": \"546c7fe2-0da6-4e7a-975b-1e752a88b092\"}, {\"host\": \"10.0.0.141\", \"uuid\": \"aac58a32-58be-4cf6-b65c-42d35d064d16\"}]");

    FlockerControlServiceClient *client = new FlockerControlServiceClient("192.168.1.101", 80);

    const Try<string> &nodeId = client->parseNodeId(json);

    ASSERT_EQ(nodeId.get(), "fef7fa02-c8c2-4c52-96b5-de70a8ef1925");
}

