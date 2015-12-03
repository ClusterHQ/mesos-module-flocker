#include <glog/logging.h>
#include <stout/foreach.hpp>
#include <stout/strings.hpp>
#include "FlockerEnvironmentalVariables.h"

using namespace mesos;

Option<FlockerEnvironmentalVariables> FlockerEnvironmentalVariables::parse(const ExecutorInfo &executorInfo) {
    if (!executorInfo.command().has_environment()) {
        LOG(INFO) << "No environment specified for container. Not a Mesos-Flocker application. ";
        return None();
    }

    FlockerEnvironmentalVariables envVars = FlockerEnvironmentalVariables(executorInfo);
    if (envVars.parseError) {
        return None();
    } else {
        return envVars;
    }
}

Option<string> FlockerEnvironmentalVariables::getUserDir() {
    return this->userDir;
}

Option<string> FlockerEnvironmentalVariables::getUserFlockerId() {
    return this->userFlockerId;
}

FlockerEnvironmentalVariables::FlockerEnvironmentalVariables(const ExecutorInfo &executorInfo) {
    foreach (const auto &variable, executorInfo.command().environment().variables()) {
        if (strings::startsWith(variable.name(), FlockerEnvironmentalVariables::FLOCKER_CONTAINER_VOLUME_PATH)) {
            this->userDir = variable.value();
            LOG(INFO) << "Container volume name ("
            << this->userDir.getOrElse("NO_DIR")
            << ") parsed from environment";
        } else if (strings::startsWith(variable.name(), FlockerEnvironmentalVariables::FLOCKER_ID)) {
            this->userFlockerId = variable.value();
            LOG(INFO) << "Using "
            << FlockerEnvironmentalVariables::FLOCKER_ID
            << " ("
            << this->userFlockerId.getOrElse("NO ID")
            << ") parsed from environment";
        }
        // Add more parsers here.
    }

    if (this->getUserDir().isNone()) {
        LOG(ERROR) << "Could not parse" << FLOCKER_CONTAINER_VOLUME_PATH <<
        "from environmental variables. Not a Mesos-Flocker application";
        this->parseError = true;
    }
    if (this->getUserFlockerId().isNone()) {
        LOG(WARNING)
        << FlockerEnvironmentalVariables::FLOCKER_ID
        << " not specified. You will not be able to migrate data on failover.";
    }
}
