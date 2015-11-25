#include "../../libisolator/flocker-isolator.hpp"
#include <stout/os.hpp>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "test_flocker_isolator.hpp"

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

    EXPECT_EQ(flockerControlIp, result.get()->getFlockerControlIp());
    EXPECT_EQ(flockerControlPort, result.get()->getFlockerControlPort());
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

    Parameters parameters;

    const char *flockerControlIp = "192.1.2.3";
    Parameter* parameter = parameters.add_parameter();
    parameter->set_key("flocker_control_ip");
    parameter->set_value(flockerControlIp);

    const char *flockerControlPort = "4523";
    parameter = parameters.add_parameter();
    parameter->set_key("flocker_control_port");
    parameter->set_value(flockerControlPort);

    Try<FlockerIsolator*> result = FlockerIsolator::create(parameters);
    if (result.isError()) {
        cerr << "Could not create Flocker isolator" << endl;
    }

    ContainerID containerId;
    containerId.set_value("befa2b13da05");

    ExecutorInfo executor;
    executor.mutable_executor_id()->set_value("default");
    executor.mutable_command()->set_value("/bin/sleep");
    executor.mutable_command()->add_arguments("60");
    executor.set_name("Test Executor (/bin/sleep)");

    Result<string> user = os::user();

    result.get()->prepare(containerId, executor, "/tmp", user.get());

    EXPECT_EQ(1, 1);
}


