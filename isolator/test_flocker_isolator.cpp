#include "flocker-isolator.hpp"
#include <stout/os.hpp>


using namespace mesos::slave;
using namespace mesos;
using namespace std;

int main() {
    Parameters parameters;
    Parameter* parameter = parameters.add_parameter();
    parameter->set_key("foo");
    parameter->set_value("foovalue");

    Try<Isolator*> result = FlockerIsolator::create(parameters);
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
}

