#include "flocker-isolator.hpp"
#include "flocker_control_service_client.hpp"
#include <mesos/module.hpp>
#include <mesos/module/isolator.hpp>
#include <stout/os/posix/shell.hpp>
#include <stout/try.hpp>

using namespace mesos::slave;
using namespace process;

using mesos::slave::ContainerPrepareInfo;
using mesos::slave::ContainerLimitation;

namespace http = process::http;

const char  FlockerIsolator::prohibitedchars[NUM_PROHIBITED]  = {
        '%', '/', ':', ';', '\0',
        '<', '>', '|', '`', '$', '\'',
        '?', '^', '&', ' ', '{', '\"',
        '}', '[', ']', '\n', '\t', '\v', '\b', '\r', '\\' };

 FlockerIsolator::FlockerIsolator(const std::string flockerControlIp, uint16_t flockerControlPort) {
    this->flockerControlServiceClient = new FlockerControlServiceClient(flockerControlIp, flockerControlPort);
 }

 FlockerIsolator::~ FlockerIsolator() {}

Try<mesos::slave::FlockerIsolator*>  FlockerIsolator::create(const Parameters& parameters)
{
    LOG(INFO) << "Creating FlockerIsolator";

    if (parameters.parameter_size() != 2) {
        std::cerr << "Could not initialize FlockerIsolator. Specify Flocker Control Service IP and port as parameters 1 and 2 respectively" << std::endl;
        return Error("Could not initialize FlockerIsolator. Specify Flocker Control Service IP and port as parameters 1 and 2 respectively");
    }

    // TODO: Validate IP address

    Try<uint16_t> flockerControlPort = numify<uint16_t>(parameters.parameter(1).value());
    if (!flockerControlPort.isSome()) {
        std::cerr << "Could not initialize FlockerIsolator. Specify Flocker Control Service IP and port as parameters 1 and 2 respectively" << std::endl;
        return Error("Could not initialize FlockerIsolator. Specify Flocker Control Service IP and port as parameters 1 and 2 respectively");
    }

    return new FlockerIsolator(parameters.parameter(0).value(), flockerControlPort.get());
}

Future<Nothing>  FlockerIsolator::recover(
        const std::list<ContainerState>& states,
        const hashset<ContainerID>& orphans)
{
    LOG(INFO) << " FlockerIsolator recover() was called";
    return Nothing();
}

Future<Option<ContainerPrepareInfo>>  FlockerIsolator::prepare(
        const ContainerID& containerId,
        const ExecutorInfo& executorInfo,
        const std::string& directory,
        const Option<std::string>& user)
{
    LOG(INFO) << "Preparing external storage for container: " << stringify(containerId);

    Try<std::string> resultJson = flockerControlServiceClient->getNodeId();
    if (resultJson.isError()) {
        std::cerr << "Could not get node id for container: " << containerId << endl;
        return Failure("Could not create node if for container: " + containerId.value());
    }

    // TODO: Parse uuid from nodeIdJson
    UUID uuid = UUID::fromString("546c7fe2-0da6-4e7a-975b-1e752a88b092");

    Try<std::string> datasetJson = flockerControlServiceClient->createDataSet(uuid);
    if (datasetJson.isError()) {
        std::cerr << "Could not create dataset for container: " << containerId << endl;
        return Failure("Could not create dataset for container: " + containerId.value());
    }

    std::string datasetUUID = flockerControlServiceClient->getFlockerDataSetUUID(datasetJson.get());

    LOG(INFO) << datasetUUID;

    // TODO: Create container prepare info

    // TODO: Create mount

    return None();
}

process::Future<Nothing>  FlockerIsolator::isolate(
        const ContainerID& containerId,
        pid_t pid)
{
    LOG(INFO) << "Isolate containerId: " << containerId << " and pid: " << pid;

    // No-op, isolation happens when mounting/unmounting in prepare/cleanup
    return Nothing();
}

process::Future<ContainerLimitation>  FlockerIsolator::watch(
        const ContainerID& containerId)
{
    LOG(INFO) << "Watch containerId: " << containerId;

    // No-op, for now.
    return process::Future<ContainerLimitation>();
}

process::Future<Nothing>  FlockerIsolator::update(
        const ContainerID& containerId,
        const Resources& resources)
{
    LOG(INFO) << containerId << " is updated";

    // No-op, isolation happens when mounting/unmounting in prepare/cleanup
    return Nothing();
}

process::Future<mesos::ResourceStatistics>  FlockerIsolator::usage(
        const ContainerID& containerId)
{
    // No-op, no usage gathered.
    return ResourceStatistics();
}

process::Future<Nothing>  FlockerIsolator::cleanup(
        const ContainerID& containerId)
{
    // No-op, isolation happens when mounting/unmounting in prepare/cleanup
    return Nothing();
}

bool  FlockerIsolator::unmount(const  FlockerIsolator::ExternalMount &em,
                                     const std::string &callerLabelForLogging) const
{
    LOG(INFO) << em << " is being unmounted on " << callerLabelForLogging;
    return true;
}

std::string  FlockerIsolator::mount(const  FlockerIsolator::ExternalMount &em,
                                          const std::string &callerLabelForLogging) const
{
    LOG(INFO) << em << " is being mounted on " << callerLabelForLogging;
    std::string mountpoint; // return value init'd to empty
    return mountpoint;
}

bool  FlockerIsolator::containsProhibitedChars(const std::string &s) const
{
    return (std::string::npos != s.find_first_of( FlockerIsolator::prohibitedchars, 0,  FlockerIsolator::NUM_PROHIBITED));
}

std::ostream & FlockerIsolator::dumpInfos(std::ostream &out) const
{
    return out;
}

static Isolator* createFlockerIsolator(const mesos::Parameters& parameters)
{
    LOG(INFO) << "Loading Flocker Mesos Isolator module";

    Try<FlockerIsolator*> result =  FlockerIsolator::create(parameters);

    return result.get();
}

// Declares the isolator named com_clusterhq_flocker_ FlockerIsolator
mesos::modules::Module<Isolator> com_clusterhq_flocker_FlockerIsolator(
        MESOS_MODULE_API_VERSION,
        MESOS_VERSION,
        "clusterhq{code}",
        "info@clusterhq.com",
        "Mesos Flocker Isolator module.",
        NULL,
        createFlockerIsolator);

FlockerControlServiceClient *FlockerIsolator::getFlockerControlClient() {
    return flockerControlServiceClient;
}
