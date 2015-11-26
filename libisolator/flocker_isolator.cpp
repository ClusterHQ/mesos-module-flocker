#include "flocker-isolator.hpp"
#include <mesos/module.hpp>
#include <mesos/module/isolator.hpp>

using namespace mesos::slave;
using mesos::slave::ContainerPrepareInfo;
using mesos::slave::ContainerLimitation;

namespace http = process::http;

const char  FlockerIsolator::prohibitedchars[NUM_PROHIBITED]  = {
        '%', '/', ':', ';', '\0',
        '<', '>', '|', '`', '$', '\'',
        '?', '^', '&', ' ', '{', '\"',
        '}', '[', ']', '\n', '\t', '\v', '\b', '\r', '\\' };

 FlockerIsolator::FlockerIsolator(const std::string flockerControlIp, uint16_t flockerControlPort) {
    this->flockerControlIp = flockerControlIp;
    this->flockerControlPort = flockerControlPort;
 }

 FlockerIsolator::~ FlockerIsolator() {}

Try<mesos::slave::FlockerIsolator*>  FlockerIsolator::create(const Parameters& parameters)
{
    LOG(INFO) << "Create isolator process";

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

process::Future<Nothing>  FlockerIsolator::recover(
        const std::list<ContainerState>& states,
        const hashset<ContainerID>& orphans)
{
    LOG(INFO) << " FlockerIsolator recover() was called";
    return Nothing();
}

process::Future<Option<ContainerPrepareInfo>>  FlockerIsolator::prepare(
        const ContainerID& containerId,
        const ExecutorInfo& executorInfo,
        const std::string& directory,
        const Option<std::string>& user)
{
    LOG(INFO) << "Preparing external storage for container: " << stringify(containerId);
    LOG(INFO) << "ExecutorInfo: " << stringify(executorInfo);
    LOG(INFO) << "Directory: " << directory;

    // Get things we need from task's environment in ExecutorInfo.
    if (!executorInfo.command().has_environment()) {
        // No environment means no external volume specification.
        // Not an error, just nothing to do, so return None.
        LOG(INFO) << "No environment specified for container ";
        return None();
    }
    std::string userDir;

    // Iterate through the environment variables,
    // looking for the ones we need.
            foreach (const auto &variable,
                     executorInfo.command().environment().variables()) {
                    if (strings::startsWith(variable.name(), FLOCKER_CONTAINER_VOLUME_PATH)) {
                        userDir = variable.value();
                        LOG(INFO) << "Container volume name ("
                        << userDir
                        << ") parsed from environment";
                    }
                }

    if (userDir.empty()) {
        LOG(ERROR) << "Could not parse FLOCKER_CONTAINER_VOLUME_PATH from environmental variables.";
        return None();
    }

    // Determine the source of the mount.
    std::string flockerDir = path::join("/flocker",
                                        "test"); // This should be the returned flocker ID: /flocker/${FLOCKER_UUID}

    // If the user dir doesn't exist on the host, create.
    if (!os::exists(userDir)) {
        Try<Nothing> mkdir = os::mkdir(userDir);
        if (mkdir.isError()) {
            LOG(ERROR) << "Failed to create the target of the mount at '" +
                          userDir << "': " << mkdir.error();
            return None();
        }
    }

    // Run the bind command
    Try<std::string> retcode = os::shell("%s %s %s",
                                         "mount --rbind", // Do we need -n here? Do we want it to appear in /etc/mtab?
                                         flockerDir.c_str(),
                                         userDir.c_str());

    if (retcode.isError()) {
        LOG(ERROR) << "mount --rbind" << " failed to execute on "
        << flockerDir
        << retcode.error();
    } else {
        LOG(INFO) << "mount --rbind" << " mounted on:"
        << userDir;

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

uint16_t FlockerIsolator::getFlockerControlPort() {
    return flockerControlPort;
}

std::string FlockerIsolator::getFlockerControlIp() {
    return flockerControlIp;
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

