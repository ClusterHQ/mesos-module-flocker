
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

// TODO: process() should be used instead of system(), but desire for
// synchronous dvdcli mount/unmount could make this complex.

#include <fstream>
#include <list>
#include <array>
#include <iostream>
#include <sstream>

#include <mesos/mesos.hpp>
#include <mesos/module.hpp>
#include <mesos/module/isolator.hpp>
#include "flocker-isolator.hpp"

#include <process/subprocess.hpp>

#include "linux/fs.hpp"

using namespace mesos::internal;

using namespace process;

using std::list;
using std::set;
using std::string;
using std::array;

using namespace mesos;
using namespace mesos::slave;

using mesos::slave::ExecutorRunState;
using mesos::slave::Isolator;
using mesos::slave::IsolatorProcess;
using mesos::slave::Limitation;

const char FlockerIsolatorProcess::prohibitedchars[NUM_PROHIBITED]  = {
'%', '/', ':', ';', '\0',
'<', '>', '|', '`', '$', '\'',
'?', '^', '&', ' ', '{', '\"',
'}', '[', ']', '\n', '\t', '\v', '\b', '\r', '\\' };

std::string FlockerIsolatorProcess::mountJsonFilename;

FlockerIsolatorProcess::FlockerIsolatorProcess(
	const Parameters& _parameters)
  : parameters(_parameters) {}

Try<Isolator*> FlockerIsolatorProcess::create(const Parameters& parameters)
{
  return new Isolator(process);
}

FlockerIsolatorProcess::~FlockerIsolatorProcess() {}

Future<Nothing> FlockerIsolatorProcess::recover(
    const list<ExecutorRunState>& states,
    const hashset<ContainerID>& orphans)
{
  LOG(INFO) << "FlockerIsolatorProcess recover() was called";

  return Nothing();
}

// Attempts to unmount specified external mount, returns true on success
// Also returns true so long as DVDCLI is successfully invoked,
// even if a non-zero return code occurs
bool FlockerIsolatorProcess::unmount(
    const ExternalMount& em,
    const std::string&   callerLabelForLogging ) const
{
    LOG(INFO) << em << " is being unmounted on " << callerLabelForLogging;

    return true;
}

// Attempts to mount specified external mount, returns true on success
std::string FlockerIsolatorProcess::mount(
    const ExternalMount& em,
    const std::string&   callerLabelForLogging) const
{
    LOG(INFO) << em << " is being mounted on " << callerLabelForLogging;

    std::string mountpoint; // return value init'd to empty

    return mountpoint;
}

std::ostream& FlockerIsolatorProcess::dumpInfos(std::ostream& out) const
{
  return out;
}

bool FlockerIsolatorProcess::containsProhibitedChars(const std::string& s) const
{
  return (string::npos != s.find_first_of(prohibitedchars, 0, NUM_PROHIBITED));
}

// Prepare runs BEFORE a task is started
// will check if the volume is already mounted and if not,
// will mount the volume
// A container can ask for multiple mounts, but if
// there are any problems parsing or mounting even one
// mount, we want to exit with an error and no new
// mounted volumes. Goal: make all mounts or none.
Future<Option<CommandInfo>> FlockerIsolatorProcess::prepare(
    const ContainerID& containerId,
    const ExecutorInfo& executorInfo,
    const string& directory,
    const Option<string>& rootfs,
    const Option<string>& user)
{
  LOG(INFO) << "Preparing external storage for container: "
            << stringify(containerId);

  return None();
}

Future<Limitation> FlockerIsolatorProcess::watch(
    const ContainerID& containerId)
{
  // No-op, for now.

  return Future<Limitation>();
}

Future<Nothing> FlockerIsolatorProcess::update(
    const ContainerID& containerId,
    const Resources& resources)
{
  // No-op, nothing enforced.

  return Nothing();
}


Future<ResourceStatistics> FlockerIsolatorProcess::usage(
    const ContainerID& containerId)
{
  // No-op, no usage gathered.

  return ResourceStatistics();
}

process::Future<Nothing> FlockerIsolatorProcess::isolate(
    const ContainerID& containerId,
    pid_t pid)
{
  // No-op, isolation happens when mounting/unmounting in prepare/cleanup
  return Nothing();
}

Future<Nothing> FlockerIsolatorProcess::cleanup(
    const ContainerID& containerId)
{
  //    1. Get driver name and volume list from infos
  //    2. Iterate list and perform unmounts

  return Nothing();

}

static Isolator* createFlockerIsolator(const Parameters& parameters)
{
  LOG(INFO) << "Loading Flocker Mesos Isolator module";

  Try<Isolator*> result = FlockerIsolatorProcess::create(parameters);

  return result.get();
}

// Declares the isolator named com_clusterhq_flocker_FlockerIsolatorProcess
mesos::modules::Module<Isolator> com_clusterhq_flocker_FlockerIsolatorProcess(
    MESOS_MODULE_API_VERSION,
    MESOS_VERSION,
    "clusterhq{code}",
    "info@clusterhq.com",
    "Mesos Flocker Isolator module.",
    NULL,
    createFlockerIsolator);
