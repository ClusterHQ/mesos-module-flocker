#ifndef MESOS_MODULE_FLOCKER_HTTPS_CLIENT_H
#define MESOS_MODULE_FLOCKER_HTTPS_CLIENT_H

#include <string>
#include <stout/try.hpp>
#include <stout/uuid.hpp>
#include <glog/logging.h>


using namespace std;

/**
 * Client for interacting with the Flocker Control Service.
 *
 * Supports retrieving the node id and creating a dataset.
 */
class FlockerControlServiceClient
{
public:

    FlockerControlServiceClient(const string flockerControlIp, uint16_t flockerControlPort);

    virtual Try<string> getNodeId();

    virtual Try<string> createDataSet(UUID uuid);

    uint16_t getFlockerControlPort();

    string getFlockerControlIp();

    string getFlockerDataSetUUID(string);

private:

    std::string flockerControlIp;

    uint16_t flockerControlPort;

};

#endif //MESOS_MODULE_FLOCKER_HTTPS_CLIENT_H
