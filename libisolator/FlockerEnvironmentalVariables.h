#ifndef MESOS_MODULE_FLOCKER_FLOCKERENVIRONMENTALVARIABLES_H
#define MESOS_MODULE_FLOCKER_FLOCKERENVIRONMENTALVARIABLES_H

#include <mesos/mesos.pb.h>

using namespace std;
using namespace mesos;

/**
 * Parses environmental variables from ExecutorInfo.
 */
class FlockerEnvironmentalVariables {
public:
    static constexpr const char *FLOCKER_CONTAINER_VOLUME_PATH = "FLOCKER_CONTAINER_VOLUME_PATH";

    static Option<FlockerEnvironmentalVariables> parse(const ExecutorInfo &executorInfo);

    Option<string> getUserDir();

private:
    FlockerEnvironmentalVariables(const ExecutorInfo &executorInfo);
    Option<string> userDir = None();
    bool parseError = false;
};

#endif //MESOS_MODULE_FLOCKER_FLOCKERENVIRONMENTALVARIABLES_H
