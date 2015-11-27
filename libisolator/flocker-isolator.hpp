
/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef SRC_FLOCKER_ISOLATOR_HPP_
#define SRC_FLOCKER_ISOLATOR_HPP_
static const char *const FLOCKER_CONTAINER_VOLUME_PATH = "FLOCKER_CONTAINER_VOLUME_PATH";

#include <iostream>
#include <boost/functional/hash.hpp>
#include <mesos/mesos.hpp>
#include <mesos/slave/isolator.hpp>
#include <slave/flags.hpp>
#include <process/future.hpp>
#include <process/owned.hpp>
#include <process/process.hpp>

#include <stout/multihashmap.hpp>
#include <stout/protobuf.hpp>
#include <stout/try.hpp>
#include "flocker_control_service_client.hpp"

namespace mesos {
namespace slave {

class  FlockerIsolator: public mesos::slave::Isolator {
public:
  static Try<mesos::slave::FlockerIsolator*> create(const Parameters& parameters);

  virtual ~ FlockerIsolator();

    // Recover containers from the run states and the orphan containers
    // (known to the launcher but not known to the slave) detected by
    // the launcher.
    virtual process::Future<Nothing> recover(
            const std::list<ContainerState>& states,
            const hashset<ContainerID> &orphans);

    // Prepare for isolation of the executor. Any steps that require
    // execution in the containerized context (e.g. inside a network
    // namespace) can be returned in the optional CommandInfo and they
    // will be run by the Launcher.
    // Prepare runs BEFORE a task is started
    // will check if the volume is already mounted and if not,
    // will mount the volume
    //
    // 1. Get Flocker Control Service IP and PORT from environment
    //    variables.
    // 2. POST request to Flocker Control Service.
    // 3. Poll Flocker Control Service waiting for volume to manifest.
    //    Note: please update "executor_registration_timeout" flag
    //    in case supported backends are expected to take more than
    //    default 1 minute timeout while manifesting datasets (EBS,
    //    for example, takes up to 360 seconds for attaching a volume).
    //    Ref: http://mesos.apache.org/documentation/latest/configuration/
    // 4. GET volume's mounted path (/flocker/uuid).
    // 5. Add entry to hashmap that contains root mountpath indexed by ContainerId
    virtual process::Future<Option<ContainerPrepareInfo>> prepare(
            const ContainerID& containerId,
            const ExecutorInfo& executorInfo,
            const std::string& directory,
            const Option<std::string> &user);

    // Isolate the executor.
    // Nothing will be done at task start
    virtual process::Future<Nothing> isolate(
            const ContainerID& containerId,
            pid_t pid);

    // Watch the containerized executor and report if any resource
    // constraint impacts the container, e.g., the kernel killing some
    // processes.
    // no-op, mount occurs at prepare
    virtual process::Future<ContainerLimitation> watch(
            const ContainerID &containerId);

    // Update the resources allocated to the container.
    // no-op, no usage stats gathered
    virtual process::Future<Nothing> update(
            const ContainerID& containerId,
            const Resources &resources);

    // Gather resource usage statistics for the container.
    virtual process::Future<ResourceStatistics> usage(
            const ContainerID &containerId);

    // Clean up a terminated container. This is called after the
    // executor and all processes in the container have terminated.
    // no-op, lazy unmount when necessary
    virtual process::Future<Nothing> cleanup(
            const ContainerID &containerId);

    FlockerControlServiceClient* getFlockerControlClient();

private:

  FlockerIsolator(const std::string flockerControlIp, uint16_t flockerControlPort);

  const Parameters parameters;

  using ExternalMountID = size_t;

  struct ExternalMount
  {
    explicit ExternalMount(
        const std::string& _volumeId,
		const std::string& _mountOptions)
      : volumeId(_volumeId),
		mountOptions(_mountOptions),
		mountpoint() {}

    explicit ExternalMount(
        const std::string& _volumeId,
		const std::string& _mountOptions,
        const std::string& _mountpoint)
      : mountOptions(_mountOptions),
		mountpoint(_mountpoint) {}

    bool operator ==(const ExternalMount& other) {
    	return getExternalMountId() == other.getExternalMountId();
    }

    ExternalMountID getExternalMountId(void) const {
        size_t seed = 0;
        std::string s1(volumeId);
        boost::hash_combine(seed, s1);
        return seed;
    }

    // We save the full root path of any mounted device here.
    // note device driver name and volume name are not case sensitive,
    // but are stored as submitted in constructor
    const std::string volumeId;
    const std::string mountOptions;
    const std::string mountpoint;
  };

  // overload '<<' operator to allow rendering of ExternalMount
  friend inline std::ostream& operator<<(std::ostream& os, const ExternalMount& em)
  {
    os << em.volumeId << '(' << em.mountOptions << ") " << em.mountpoint;
    return os;
  }

  // Attempts to unmount specified external mount, returns true on success
  bool unmount(
      const ExternalMount& em,
      const std::string&   callerLabelForLogging ) const;

  // Attempts to mount specified external mount,
  // returns non-empty string on success
  std::string mount(
      const ExternalMount& em,
      const std::string&   callerLabelForLogging) const;

  // Returns true if string contains at least one prohibited character
  // as defined in the list below.
  // This is intended as a tool to detect injection attack attempts.
  bool containsProhibitedChars(const std::string& s) const;

  std::ostream& dumpInfos(std::ostream& out) const;

  using containermountmap =
    multihashmap<ContainerID, process::Owned<ExternalMount>>;
  containermountmap infos;

  // compiler had issues with the autodetecting size of following array,
  // thus a constant is defined

  static constexpr size_t NUM_PROHIBITED = 26;
  static const char prohibitedchars[NUM_PROHIBITED]; /*  = {
  '%', '/', ':', ';', '\0',
  '<', '>', '|', '`', '$', '\'',
  '?', '^', '&', ' ', '{', '\"',
  '}', '[', ']', '\n', '\t', '\v', '\b', '\r', '\\' };*/

  static constexpr const char* FLOCKER_MOUNT_PREFIX       = "/flocker/";

  static constexpr const char* FLOCKER_MOUNTLIST_DEFAULT_DIR = "/var/lib/mesos/flocker/";
  static constexpr const char* FLOCKER_MOUNTLIST_FILENAME   = "flockermounts.json";
  static constexpr const char* FLOCKER_WORKDIR_PARAM_NAME   = "work_dir";

  static std::string mountJsonFilename;

  FlockerControlServiceClient *flockerControlServiceClient;
};

} /* namespace slave */
} /* namespace mesos */

#endif /* SRC_FLOCKER_ISOLATOR_HPP_ */
