#ifndef MESOS_MODULE_FLOCKER_FLOCKERENVIRONMENTALVARIABLES_H
#define MESOS_MODULE_FLOCKER_FLOCKERENVIRONMENTALVARIABLES_H

#include <mesos/mesos.pb.h>

using namespace std;
using namespace mesos;

class FlockerEnvironmentalVariables {
public:
    static constexpr const char *FLOCKER_CONTAINER_VOLUME_PATH = "FLOCKER_CONTAINER_VOLUME_PATH";

    Option<FlockerEnvironmentalVariables> parse(const ExecutorInfo &executorInfo);

    Option<string> getUserDir();

private:
    FlockerEnvironmentalVariables(const ExecutorInfo &executorInfo);

    Option<string> userDir = None();
};

#endif //MESOS_MODULE_FLOCKER_FLOCKERENVIRONMENTALVARIABLES_H
