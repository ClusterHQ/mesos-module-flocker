#ifndef MESOS_MODULE_FLOCKER_HTTPS_CLIENT_H
#define MESOS_MODULE_FLOCKER_HTTPS_CLIENT_H

#include <string>
#include <stout/try.hpp>
#include <stout/uuid.hpp>
#include <glog/logging.h>
#include "IpUtils.hpp"

using namespace std;

/**
 * Client for interacting with the Flocker Control Service.
 *
 * Supports retrieving the node id and creating a dataset.
 */
class FlockerControlServiceClient
{
public:

    FlockerControlServiceClient(const string flockerControlIp, uint16_t flockerControlPort, IpUtils *ipUtils);

    virtual Try<string> getNodeId();

    virtual Try<string> createDataSet(UUID uuid, string flockerId);

    uint16_t getFlockerControlPort();

    string getFlockerControlIp();

    string getFlockerDataSetUUID(string);

    Try<string> parseNodeId(Try<string> jsonNodes);

    Option<string> getDataSetForFlockerId(string flockerId);

    Option<string> parseDataSet(Try<string> aTry, string flockerId);

    Try<string> moveDataSet(string option, UUID uuid);

private:

    std::string flockerControlIp;

    uint16_t flockerControlPort;

    IpUtils *ipUtils;

};

#endif //MESOS_MODULE_FLOCKER_HTTPS_CLIENT_H
