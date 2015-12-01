#include "flocker_control_service_client.hpp"

#include <stout/format.hpp>

#include <stout/os/posix/shell.hpp>

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <stout/nothing.hpp>
#include <stout/json.hpp>

using namespace std;

FlockerControlServiceClient::FlockerControlServiceClient(const string flockerControlIp, uint16_t flockerControlPort) {
    this->flockerControlIp      = flockerControlIp;
    this->flockerControlPort    = flockerControlPort;
}


string FlockerControlServiceClient::getIpAddress() {
    int fd;
    struct ifreq ifr;

    fd = socket(AF_INET, SOCK_DGRAM, 0);

    /* I want to get an IPv4 IP address */
    ifr.ifr_addr.sa_family = AF_INET;

    /* I want IP address attached to "eth0" */
    strncpy(ifr.ifr_name, "eth0", IFNAMSIZ-1);

    ioctl(fd, SIOCGIFADDR, &ifr);

    close(fd);

    return inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr);
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
    Try<string> curlCommand = os::shell("curl -XGET -H \"Content-Type: application/json\" --cacert /etc/flocker/cluster.crt --cert /etc/flocker/plugin.crt --key /etc/flocker/plugin.key https://" + flockerControlIp + ":" + stringify(flockerControlPort) + "/v1/state/nodes");
    LOG(INFO) << curlCommand.isSome();
    if (curlCommand.isSome()) {
        string curlResut = curlCommand.get();
        LOG(INFO) << curlResut;
        string ipAddress = getIpAddress();
        ipAddress = "\"" + ipAddress + "\"";
        LOG(INFO) << ipAddress;
        unsigned long idLoc = curlResut.find(ipAddress);
        LOG(INFO) << idLoc;
        unsigned long firstQuoteLoc = curlResut.find("\"uuid\": \"", idLoc + ipAddress.length()) + strlen("\"uuid\": \"");
        LOG(INFO) << firstQuoteLoc;
        unsigned long lastQuoteLoc = curlResut.find("\"", firstQuoteLoc + strlen("\"uuid\": \""));
        LOG(INFO) << lastQuoteLoc;
        string retVal = curlResut.substr(firstQuoteLoc, lastQuoteLoc - firstQuoteLoc);
        LOG(INFO) << retVal;
        return retVal;
    } else {
        return Error("Unable to curl node state");
    }
}
