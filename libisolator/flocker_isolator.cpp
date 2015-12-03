#include "flocker-isolator.hpp"
#include "FlockerEnvironmentalVariables.h"
#include <mesos/module.hpp>
#include <mesos/module/isolator.hpp>
#include <stout/os/posix/exists.hpp>
#include <stout/os/posix/shell.hpp>

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

 FlockerIsolator::FlockerIsolator(FlockerControlServiceClient *flockerControlServiceClient) {
    this->flockerControlServiceClient = flockerControlServiceClient;
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

    FlockerControlServiceClient *client = new FlockerControlServiceClient(parameters.parameter(0).value(), flockerControlPort.get(), new IpUtils());
    return new FlockerIsolator(client);
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
    LOG(INFO) << "ExecutorInfo: " << stringify(executorInfo);
    LOG(INFO) << "Directory: " << directory;

    // *****************
    // Read user directory from environmental variables.
    Option<FlockerEnvironmentalVariables> envVars = FlockerEnvironmentalVariables::parse(executorInfo);
    if (envVars.isNone()) {
        LOG(INFO) << "No environment specified for container. Not a Mesos-Flocker application. ";
        return None();
    }
    LOG(INFO) << "Parsed env vars" << endl;

    // If the user dir already exists, bail out.
    if (os::exists(envVars->getUserDir().get())) {
        LOG(ERROR) << "User dir already exists. Stopping to prevent data loss. " << envVars->getUserDir().get();
        return Failure("User dir already exists");
    }

    // *****************
    // Send REST command to Flocker to get the current Flocker node ID
    Try<std::string> nodeId = flockerControlServiceClient->getNodeId();
    if (nodeId.isError()) {
        return Failure("Could not get node id for container: " + containerId.value());
    } else {
        LOG(INFO) << "Got node UUID: " << nodeId.get() << endl;
    }

    const UUID &nodeUUID = UUID::fromString(nodeId.get());

    Option<string> datasetUUID = flockerControlServiceClient->getDataSetForFlockerId(envVars->getUserFlockerId());

    if (datasetUUID.isNone()) {
        // *****************
        // Send REST command to Flocker to create a new dataset
        Try<std::string> datasetJson = flockerControlServiceClient->createDataSet(nodeUUID);
        if (datasetJson.isError()) {
            std::cerr << "Could not create dataset for container: " << containerId << endl;
            return Failure("Could not create dataset for container: " + containerId.value());
        } else {
            LOG(INFO) << "Created dataset: " << datasetJson.get() << endl;
        }

        Try<JSON::Object> parse = JSON::parse<JSON::Object>(datasetJson.get());
        if (parse.isError()) {
            std::cerr << "Could not parse JSON" << endl;
            return Failure("Could not create node if for container: " + containerId.value());
        }
        LOG(INFO) << "Parsed JSON: " << parse.get() << endl;

        // Determine the source of the mount.
        std::string flockerDir = path::join("/flocker", datasetUUID.get()); // This should be the returned flocker ID: /flocker/${FLOCKER_UUID}

        LOG(INFO) << "Waiting for" << datasetUUID.get() << "to mount";

        // *****************
        // Wait for the flocker dataset to mount
        unsigned long watchdog = 0;
        while ((!os::exists(flockerDir))) {
            usleep(1000000); // Sleep for 1 s.
            if (watchdog++ > 60) {
                LOG(ERROR) << "Flocker did not mount within 60 s" << containerId << endl;
                return Failure("Flocker did not mount within 60 s: " + containerId.value());
            }
        }

        // *****************
        // Symlink to the flocker dir
        // sudo ln -s /flocker/7dfae970-fb3f-4ca3-8d9a-a93271d8f1e3 /tmp/data
        Try<std::string> retcode = os::shell("%s %s %s",
                                             "ln -s", // Do we need -n here? Do we want it to appear in /etc/mtab?
                                             flockerDir.c_str(),
                                             envVars->getUserDir().get().c_str());

        if (retcode.isError()) {
            LOG(ERROR) << "mount --rbind" << " failed to execute on "
            << flockerDir
            << retcode.error();
        } else {
            LOG(INFO) << "mount --rbind" << " mounted on:"
            << envVars->getUserDir().get();

        }
    } else {
        // *****************
        // Send REST command to Flocker to move dataset
        Try<std::string> datasetJson = flockerControlServiceClient->moveDataSet(datasetUUID.get(), nodeUUID);
        if (datasetJson.isError()) {
            std::cerr << "Could not move dataset for container: " << containerId << endl;
            return Failure("Could not move dataset for container: " + containerId.value());
        } else {
            LOG(INFO) << "Moved dataset: " << datasetJson.get() << endl;
        }

        Try<JSON::Object> parse = JSON::parse<JSON::Object>(datasetJson.get());
        if (parse.isError()) {
            std::cerr << "Could not parse JSON" << endl;
            return Failure("Could not move dataset node if for container: " + containerId.value());
        }
        LOG(INFO) << "Parsed JSON: " << parse.get() << endl;
    }

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
