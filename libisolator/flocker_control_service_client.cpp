#include "flocker_control_service_client.hpp"
#include "IpUtils.hpp"

#include <stout/format.hpp>

#include <stout/os/posix/shell.hpp>
#include <stout/json.hpp>

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <stout/nothing.hpp>
#include <stout/json.hpp>

using namespace std;

FlockerControlServiceClient::FlockerControlServiceClient(const string flockerControlIp, uint16_t flockerControlPort, IpUtils *ipUtils) {
    this->flockerControlIp      = flockerControlIp;
    this->flockerControlPort    = flockerControlPort;
    this->ipUtils               = ipUtils;
}

Try<string> FlockerControlServiceClient::createDataSet(UUID uuid, string flockerId) {
    return os::shell(
            "curl -XPOST -H \"Content-Type: application/json\" --cacert /etc/flocker/cluster.crt --cert /etc/flocker/plugin.crt --key /etc/flocker/plugin.key -d '{\"primary\": \"" +
            uuid.toString() + "\", \"metadata\": { \"" + flockerId + "\" }\"}' https://" + flockerControlIp + ":" + stringify(flockerControlPort) +
            "/v1/configuration/datasets");
}

uint16_t FlockerControlServiceClient::getFlockerControlPort() {
    return flockerControlPort;
}

string FlockerControlServiceClient::getFlockerControlIp() {
    return flockerControlIp;
}

string FlockerControlServiceClient::getFlockerDataSetUUID(string json) {
    Try<JSON::Object> parse = JSON::parse<JSON::Object>(json);
    if (!parse.isSome()) {
        std::cerr << "Could not parse JSON" << parse.get() << endl;
        return "";
    }
    LOG(INFO) << parse.get() << endl;
    JSON::Object dataSetJson = parse.get();
    const std::string path = "dataset_id";
    Result<JSON::String> datasetUUID = dataSetJson.find<JSON::String>(path);
    return datasetUUID.get().value;
}

Try<string> FlockerControlServiceClient::getNodeId() {
    Try<string> curlCommand = os::shell(buildNodesCommand());
    return parseNodeId(curlCommand);
}

string FlockerControlServiceClient::buildNodesCommand() const {
    return "curl -XGET -H \"Content-Type: application/json\" --cacert /etc/flocker/cluster.crt --cert /etc/flocker/plugin.crt --key /etc/flocker/plugin.key https://" +
           flockerControlIp + ":" + stringify(flockerControlPort) + "/v1/state/nodes";
}

Option<string> FlockerControlServiceClient::getDataSetForFlockerId(string flockerId) {
    Try<string> curlCommand = os::shell("curl -XGET -H \"Content-Type: application/json\" --cacert /etc/flocker/cluster.crt --cert /etc/flocker/plugin.crt --key /etc/flocker/plugin.key https://" + flockerControlIp + ":" + stringify(flockerControlPort) + "/v1/configuration/datasets");
    return parseDataSet(curlCommand.get(), flockerId);
}

Try<string> FlockerControlServiceClient::moveDataSet(string dataSet, const UUID nodeId) {
    return os::shell(
            "curl -XPOST -H \"Content-Type: application/json\" --cacert /etc/flocker/cluster.crt --cert /etc/flocker/plugin.crt --key /etc/flocker/plugin.key https://" +
            flockerControlIp + ":" + stringify(flockerControlPort) + " /v1/configuration/datasets/" + dataSet + "-d { \"primary:\" \"" + nodeId.toString() + "\"  \"}");
}

Try<string> FlockerControlServiceClient::parseNodeId(Try<std::string> jsonNodes) {
    Try<JSON::Array> nodeArray = JSON::parse<JSON::Array>(jsonNodes.get());
    if (!nodeArray.isSome()) {
        std::cerr << "Could not parse JSON" << nodeArray.get() << endl;
        return Error("Could not parse JSON");
    }

    const string &ipAddress = ipUtils->getIpAddress();

    LOG(INFO) << "Node IP address is: " << ipAddress << endl;

    for (int j = 0; j < nodeArray->values.size(); ++j) {
        JSON::Value value = nodeArray.get().values[j];
        const JSON::Object &object = value.as<JSON::Object>();

        LOG(INFO) << object << endl;

        const Result<JSON::String> &result = object.find<JSON::String>("host");
        LOG(INFO) << result.get() << endl;

        if (result.get() == ipAddress) {
            const Result<JSON::String> &nodeResult = object.find<JSON::String>("uuid");
            std::string nodeId = nodeResult.get().value;
            return Try<string>::some(nodeId);
        }
    }

    return Error("Unable to find node ID for node: " + ipAddress);
}

Option<string> FlockerControlServiceClient::parseDataSet(Try<string> jsonDataSets, string flockerId) {
    Try<JSON::Array> nodeArray = JSON::parse<JSON::Array>(jsonDataSets.get());
    if (!nodeArray.isSome()) {
        std::cerr << "Could not parse JSON" << nodeArray.get() << endl;
        return Option<string>::none();
    }

    for (int j = 0; j < nodeArray->values.size(); ++j) {
        JSON::Value value = nodeArray.get().values[j];
        const JSON::Object &object = value.as<JSON::Object>();

        LOG(INFO) << object << endl;

        const Result<JSON::Object> &metadata = object.find<JSON::Object>("metadata");

        if (metadata.isNone()) {
            continue;
        }


        const Result<JSON::String> &flockerIdValue = metadata->find<JSON::String>("FLOCKER_ID");
        if (flockerIdValue.isNone()) {
            continue;
        }
        LOG(INFO) << "Found metadata " << metadata.get() << endl;

        if (flockerIdValue.get() == flockerId) {
            std::string dataSetId = object.find<JSON::String>("dataset_id").get().value;
            return Option<string>::some(dataSetId);
        }
    }

    return Option<string>::none();
}
