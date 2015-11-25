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

 FlockerIsolator:: FlockerIsolator(const mesos::Parameters& parameters) {}

 FlockerIsolator::~ FlockerIsolator() {}

Try<mesos::slave::Isolator*>  FlockerIsolator::create(const Parameters& parameters)
{
    LOG(INFO) << "Create isolator process";
    return new FlockerIsolator(parameters);
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

    const http::URL url = http::URL("http", "google.com");
    process::Future<http::Response> response = process::http::get(url);

    std::cout << response.get().body;

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

    Try<Isolator*> result =  FlockerIsolator::create(parameters);

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