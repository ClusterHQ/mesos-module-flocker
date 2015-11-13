#include "flocker-isolator.hpp"
#include <mesos/module.hpp>
#include <mesos/module/isolator.hpp>

using namespace mesos::slave;
using mesos::slave::ContainerPrepareInfo;

const char FlockerIsolatorProcess::prohibitedchars[NUM_PROHIBITED]  = {
        '%', '/', ':', ';', '\0',
        '<', '>', '|', '`', '$', '\'',
        '?', '^', '&', ' ', '{', '\"',
        '}', '[', ']', '\n', '\t', '\v', '\b', '\r', '\\' };

FlockerIsolatorProcess::FlockerIsolatorProcess(const mesos::Parameters& parameters) {}

FlockerIsolatorProcess::~FlockerIsolatorProcess() {}

Try<mesos::slave::Isolator*> FlockerIsolatorProcess::create(const Parameters& parameters)
{
//    return new FlockerIsolatorProcess(parameters);
}

process::Future<Nothing> FlockerIsolatorProcess::recover(
        const std::list<ContainerState>& states,
        const hashset<ContainerID>& orphans)
{
    LOG(INFO) << "FlockerIsolatorProcess recover() was called";
    return Nothing();
}

process::Future<Option<ContainerPrepareInfo>> FlockerIsolatorProcess::prepare(
        const ContainerID& containerId,
        const ExecutorInfo& executorInfo,
        const std::string& directory,
        const Option<std::string>& user)
{
    LOG(INFO) << "Preparing external storage for container: " << stringify(containerId);
    return None();
}

process::Future<Nothing> FlockerIsolatorProcess::isolate(
        const ContainerID& containerId,
        pid_t pid)
{
    // No-op, isolation happens when mounting/unmounting in prepare/cleanup
    return Nothing();
}

process::Future<ContainerLimitation> FlockerIsolatorProcess::watch(
        const ContainerID& containerId)
{
    // No-op, for now.
    return ContainerLimitation();
}

process::Future<Nothing> FlockerIsolatorProcess::update(
        const ContainerID& containerId,
        const Resources& resources)
{
    // No-op, isolation happens when mounting/unmounting in prepare/cleanup
    return Nothing();
}

process::Future<mesos::ResourceStatistics> FlockerIsolatorProcess::usage(
        const ContainerID& containerId)
{
    // No-op, no usage gathered.
    return ResourceStatistics();
}

process::Future<Nothing> FlockerIsolatorProcess::cleanup(
        const ContainerID& containerId)
{
    // No-op, isolation happens when mounting/unmounting in prepare/cleanup
    return Nothing();
}

bool FlockerIsolatorProcess::unmount(const FlockerIsolatorProcess::ExternalMount &em,
                                     const std::string &callerLabelForLogging) const
{
    LOG(INFO) << em << " is being unmounted on " << callerLabelForLogging;
    return true;
}

std::string FlockerIsolatorProcess::mount(const FlockerIsolatorProcess::ExternalMount &em,
                                          const std::string &callerLabelForLogging) const
{
    LOG(INFO) << em << " is being mounted on " << callerLabelForLogging;
    std::string mountpoint; // return value init'd to empty
    return mountpoint;
}

bool FlockerIsolatorProcess::containsProhibitedChars(const std::string &s) const
{
    return (std::string::npos != s.find_first_of(FlockerIsolatorProcess::prohibitedchars, 0, FlockerIsolatorProcess::NUM_PROHIBITED));
}

std::ostream &FlockerIsolatorProcess::dumpInfos(std::ostream &out) const
{
    return out;
}

static Isolator* createFlockerIsolator(const mesos::Parameters& parameters)
{
    LOG(INFO) << "Loading Flocker Mesos Isolator module";

    Try<Isolator*> result = FlockerIsolatorProcess::create(parameters);

    return result.get();
}

// Declares the isolator named com_clusterhq_flocker_FlockerIsolatorProcess
mesos::modules::Module<Isolator> com_clusterhq_flocker_FlockerIsolatorProcess(
        MESOS_MODULE_API_VERSION,
        MESOS_VERSION,
        "clusterhq{code}",
        "info@clusterhq.com",
        "Mesos Flocker Isolator module.",
        NULL,
        createFlockerIsolator);