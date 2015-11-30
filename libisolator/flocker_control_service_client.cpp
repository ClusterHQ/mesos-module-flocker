#include "flocker_control_service_client.hpp"

#include <stout/format.hpp>

#include <stout/os/posix/shell.hpp>
#include <stout/json.hpp>

using namespace std;

FlockerControlServiceClient::FlockerControlServiceClient(const string flockerControlIp, uint16_t flockerControlPort) {
    this->flockerControlIp      = flockerControlIp;
    this->flockerControlPort    = flockerControlPort;
}

Try<string> FlockerControlServiceClient::getNodeId() {
    return os::shell("curl -XGET -H \"Content-Type: application/json\" --cacert /etc/flocker/cluster.crt --cert /etc/flocker/plugin.crt --key /etc/flocker/plugin.key https://" + flockerControlIp + ":" + stringify(flockerControlPort) + "/v1/state/nodes");
}

Try<string> FlockerControlServiceClient::createDataSet(UUID uuid) {
    return os::shell(
            "curl -XPOST -H \"Content-Type: application/json\" --cacert /etc/flocker/cluster.crt --cert /etc/flocker/plugin.crt --key /etc/flocker/plugin.key -d '{\"primary\": \"" +
            uuid.toString() + "\"}' https://" + flockerControlIp + ":" + stringify(flockerControlPort) +
            "/v1/configuration/datasets");
}

uint16_t FlockerControlServiceClient::getFlockerControlPort() {
    return flockerControlPort;
}

string FlockerControlServiceClient::getFlockerControlIp() {
    return flockerControlIp;
}

string FlockerControlServiceClient::getFlockerDataSetUUID(string jsonString) {
    Try<JSON::Object> parse = JSON::parse<JSON::Object>(jsonString);
    if (parse.isError()) {
        std::cerr << "Could not parse JSON" << endl;
        return "";
    }
    JSON::Object dataSetJson = parse.get();
    return dataSetJson.values["dataset_id"].as<string>();
}
