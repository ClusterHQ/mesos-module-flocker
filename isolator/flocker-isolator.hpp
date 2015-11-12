
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

namespace mesos {
namespace slave {

class FlockerIsolatorProcess: public mesos::slave::Isolator {
public:
  static Try<mesos::slave::Isolator*> create(const Parameters& parameters);

  virtual ~FlockerIsolatorProcess();

    // Recover containers from the run states and the orphan containers
    // (known to the launcher but not known to the slave) detected by
    // the launcher.
    virtual process::Future<Nothing> recover(
            const std::list<ContainerState>& states,
            const hashset<ContainerID>& orphans) = 0;

    // Prepare for isolation of the executor. Any steps that require
    // execution in the containerized context (e.g. inside a network
    // namespace) can be returned in the optional CommandInfo and they
    // will be run by the Launcher.
    // TODO(idownes): Any URIs or Environment in the CommandInfo will be ignored; only the command value is used.
    virtual process::Future<Option<ContainerPrepareInfo>> prepare(
            const ContainerID& containerId,
            const ExecutorInfo& executorInfo,
            const std::string& directory,
            const Option<std::string>& user) = 0;

    // Isolate the executor.
    virtual process::Future<Nothing> isolate(
            const ContainerID& containerId,
            pid_t pid) = 0;

    // Watch the containerized executor and report if any resource
    // constraint impacts the container, e.g., the kernel killing some
    // processes.
    virtual process::Future<ContainerLimitation> watch(
            const ContainerID& containerId) = 0;

    // Update the resources allocated to the container.
    virtual process::Future<Nothing> update(
            const ContainerID& containerId,
            const Resources& resources) = 0;

    // Gather resource usage statistics for the container.
    virtual process::Future<ResourceStatistics> usage(
            const ContainerID& containerId) = 0;

    // Clean up a terminated container. This is called after the
    // executor and all processes in the container have terminated.
    virtual process::Future<Nothing> cleanup(
            const ContainerID& containerId) = 0;

private:
  FlockerIsolatorProcess(const Parameters& parameters);

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

  static constexpr const char* FLOCKER_CONTROL_IP        = "FLOCKER_CONTROL_IP";
  static constexpr const char* FLOCKER_CONTROL_PORT      = "FLOCKER_CONTROL_PORT";

  static constexpr const char* FLOCKER_MOUNTLIST_DEFAULT_DIR = "/var/lib/mesos/flocker/";
  static constexpr const char* FLOCKER_MOUNTLIST_FILENAME   = "flockermounts.json";
  static constexpr const char* FLOCKER_WORKDIR_PARAM_NAME   = "work_dir";

  static std::string mountJsonFilename;
};

} /* namespace slave */
} /* namespace mesos */

#endif /* SRC_FLOCKER_ISOLATOR_HPP_ */
