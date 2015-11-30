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

    return FlockerEnvironmentalVariables(executorInfo);
}

Option<string> FlockerEnvironmentalVariables::getUserDir() {
    return this->userDir;
}

FlockerEnvironmentalVariables::FlockerEnvironmentalVariables(const ExecutorInfo &executorInfo) {
            foreach (const auto &variable, executorInfo.command().environment().variables()) {
                    if (strings::startsWith(variable.name(),
                                            FlockerEnvironmentalVariables::FLOCKER_CONTAINER_VOLUME_PATH)) {
                        this->userDir = variable.value();
                        LOG(INFO) << "Container volume name ("
                        << this->userDir.getOrElse("NO_DIR")
                        << ") parsed from environment";
                    }
                }
}
