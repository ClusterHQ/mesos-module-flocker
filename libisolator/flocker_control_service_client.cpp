#include "flocker_control_service_client.hpp"

#include <stout/format.hpp>

#include <stout/os/posix/shell.hpp>

using namespace std;

FlockerControlServiceClient::FlockerControlServiceClient(const string flockerControlIp, uint16_t flockerControlPort) {
    this->flockerControlIp      = flockerControlIp;
    this->flockerControlPort    = flockerControlPort;
}

Try<string> FlockerControlServiceClient::getNodeId() {
    return os::shell("curl -XGET -H \"Content-Type: application/json\" --cacert /etc/flocker/cluster.crt --cert /etc/flocker/plugin.crt --key /etc/flocker/plugin.key https://" + flockerControlIp + ":" + stringify(flockerControlPort) + "/v1/state/nodes");
}

Try<string> FlockerControlServiceClient::createDataSet(UUID uuid) {
   return os::shell("curl -XPOST -H \"Content-Type: application/json\" --cacert /etc/flocker/cluster.crt --cert /etc/flocker/plugin.crt --key /etc/flocker/plugin.key -d '{\"primary\": \"" + uuid.toString() + "\"}' https:" + flockerControlIp + ":" + stringify(flockerControlPort) + "/v1/configuration/datasets");
}

uint16_t FlockerControlServiceClient::getFlockerControlPort() {
    return flockerControlPort;
}

string FlockerControlServiceClient::getFlockerControlIp() {
    return flockerControlIp;
}

string FlockerControlServiceClient::getFlockerDataSetUUID(string string) {
    LOG(INFO) << string;
    unsigned long idLoc = string.find("\"dataset_id\":");
    LOG(INFO) << idLoc;
    unsigned long firstQuoteLoc = string.find("\"", idLoc + strlen("\"dataset_id\":"));
    LOG(INFO) << firstQuoteLoc;
    unsigned long lastQuoteLoc = string.find("\"", firstQuoteLoc + 1);
    LOG(INFO) << lastQuoteLoc;
    std::string retVal = string.substr(firstQuoteLoc + 1, lastQuoteLoc);
    LOG(INFO) << retVal;
    return retVal;
}
