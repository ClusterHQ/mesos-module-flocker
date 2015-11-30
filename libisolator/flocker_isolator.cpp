#include "flocker-isolator.hpp"
#include "FlockerEnvironmentalVariables.h"
#include <mesos/module.hpp>
#include <mesos/module/isolator.hpp>

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

    FlockerControlServiceClient *client = new FlockerControlServiceClient(parameters.parameter(0).value(), flockerControlPort.get());
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

    Option<FlockerEnvironmentalVariables> envVars = FlockerEnvironmentalVariables::parse(executorInfo);

    // *****************
    // Read user directory from environmental variables.
    if (envVars.isNone()) {
        LOG(INFO) << "No environment specified for container. Not a Mesos-Flocker application. ";
        return None();
    }

    // *****************
    // Send REST command to Flocker to get the current Flocker node ID
    Try<std::string> resultJson = flockerControlServiceClient->getNodeId();
    if (resultJson.isError()) {
        std::cerr << "Could not get node id for container: " << containerId << endl;
        return Failure("Could not create node if for container: " + containerId.value());
    }

    // TODO: Parse uuid from nodeIdJson
    UUID uuid = UUID::fromString("fef7fa02-c8c2-4c52-96b5-de70a8ef1925");

    // *****************
    // Send REST command to Flocker to create a new dataset
    Try<std::string> datasetJson = flockerControlServiceClient->createDataSet(uuid);
    if (datasetJson.isError()) {
        std::cerr << "Could not create dataset for container: " << containerId << endl;
        return Failure("Could not create dataset for container: " + containerId.value());
    }

    std::string datasetUUID = flockerControlServiceClient->getFlockerDataSetUUID(datasetJson.get());

    LOG(INFO) << datasetUUID;

    // Determine the source of the mount.
    std::string flockerDir = path::join("/flocker",
                                        datasetUUID); // This should be the returned flocker ID: /flocker/${FLOCKER_UUID}

        LOG(INFO) << "Wating for" << datasetUUID << "to mount";

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

    // If the user dir doesn't exist on the host, create.
    if (!os::exists(envVars->getUserDir().get())) {
        Try<Nothing> mkdir = os::mkdir(envVars->getUserDir().get());
        if (mkdir.isError()) {
            LOG(ERROR) << "Failed to create the target of the mount at '" +
                                  envVars->getUserDir().get() << "': " << mkdir.error();
            return process::Failure("Failed to create the target of the mount");
        }
    }

        // *****************
        // Bind user directory to Flocker volume≠
    Try<std::string> retcode = os::shell("%s %s %s",
                                         "mount --rbind", // Do we need -n here? Do we want it to appear in /etc/mtab?
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
