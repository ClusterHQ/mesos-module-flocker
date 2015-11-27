#include "flocker_control_service_client.hpp"

#include <stdarg.h> // For va_list, va_start, etc.
#include <stdio.h> // For ferror, fgets, FILE, pclose, popen.

#include <string>

#include <stout/error.hpp>
#include <stout/format.hpp>
#include <stout/try.hpp>
#include <glog/logging.h>

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

std::string FlockerControlServiceClient::getFlockerControlIp() {
    return flockerControlIp;
}
