// Licensed to the Apache Software Foundation (ASF) under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef __CGROUPS_V2_HPP__
#define __CGROUPS_V2_HPP__

#include <set>
#include <string>
#include <vector>

#include <stout/bytes.hpp>
#include <stout/nothing.hpp>
#include <stout/option.hpp>
#include <stout/try.hpp>

namespace cgroups2 {

// Root cgroup in the cgroup v2 hierarchy. Since the root cgroup has the same
// path as the root mount point its relative path is the empty string.
const std::string ROOT_CGROUP = "";

// Checks if cgroups2 is available on the system.
bool enabled();

// Mounts the cgroups2 file system at /sys/fs/cgroup. Errors if
// the cgroups v2 file system is already mounted.
Try<Nothing> mount();

// Checks if the cgroup2 file systems is mounted at /sys/fs/cgroup,
// returns an error if the mount is found at an unexpected location.
Try<bool> mounted();

// Unmounts the cgroups2 file system from /sys/fs/cgroup. Errors if
// the cgroup2 file system is not mounted at /sys/fs/cgroup. It's the
// responsibility of the caller to ensure all child cgroups have been destroyed.
Try<Nothing> unmount();

// Entrypoint into cgroups2. Checks that the host supports cgroups2
// and all of the requested subsystems, then mounts the cgroup2 filesystem
// to /sys/fs/cgroup, if not already mounted, and enables all of the requested
// subsystems.
Try<Nothing> prepare(const std::vector<std::string>& subsystems);

// Creates a cgroup off of the base hierarchy. If `cgroup` is a nested cgroup
// and any parent cgroups do not exist an error will be returned, unless
// `recursive` is set to `true`. If the cgroup already exists, an error will
// also be returned.
//
// Examples:
// - `cgroup` = "foo" will create cgroup `/sys/fs/cgroup/foo`.
// - `cgroup` = "foo/bar" will create cgroup `/sys/fs/cgroup/foo/bar` if:
//     1) cgroup `foo` exists.
//     2) cgroup `foo` does not exist and `recursive` = `true`.
//   An error will be thrown otherwise.
Try<Nothing> create(const std::string& cgroup, bool recursive = false);

// Destroys a cgroup off of the base hierarchy. Will recursively destroy
// any child cgroups. If the cgroup does not exist, an error will be returned.
// It's the responsibility of the caller to ensure all cgroups that will be
// destroyed do not have any child processes, otherwise an error will be
// returned.
Try<Nothing> destroy(const std::string& cgroup);

namespace subsystems {

// Gets the subsystems that can be controlled by the provided cgroup.
// Providing cgroups2::ROOT_CGROUP will yield the set of subsystems available
// on the host.
Try<std::set<std::string>> available(const std::string& cgroup);

// Checks if the given subsystems can be controlled by the provided cgroup.
Try<bool> available(
  const std::string& cgroup, const std::vector<std::string>& subsystems);

// Enables the given subsystems in the cgroup and disables all other subsystems.
// Errors if a requested subsystem is not available.
Try<Nothing> enable(
  const std::string& cgroup,
  const std::vector<std::string>& subsystems);

// Checks if the given subsystems are enabled in the provided cgroup.
Try<bool> enabled(
  const std::string& cgroup, const std::vector<std::string>& subsystems);

} // namespace subsystems {

namespace memory {

// Memory usage limit.
// Represents a snapshot of "memory.high" and "memory.max" value.
struct Limit
{
  // Limit representing no limit AKA "unlimited".
  static Limit max();

  // Parse a limit from a string.
  // Format:
  // """
  // max OR <bytes>
  // """
  static Try<Limit> parse(const std::string& value);

  bool operator==(const Limit& other) const;

  // Limit in bytes. None if the limit is "unlimited".
  Option<Bytes> bytes;
};

// Get the total amount of memory currently being used by the cgroup and its
// descendants.
Try<Bytes> usage(const std::string& cgroup);

// Set the minimum memory that is guaranteed to not be reclaimed under any
// conditions. If the value is larger than the parent cgroup's value, the
// parent's value is the effective value.
// Can only be set for non-root cgroups.
Try<Nothing> minimum(const std::string& cgroup, const Bytes& bytes);


// Get the minimum memory that is guaranteed to not be reclaimed under any
// conditions.
// Only exists for non-root cgroups.
Try<Bytes> minimum(const std::string& cgroup);

// Set the maximum memory that can be used by a cgroup and its descendants.
// Exceeding the limit will trigger the OOM killer.
// Can only be set for non-root cgroups.
Try<Nothing> maximum(const std::string& cgroup, const Limit& limit);

// Get the maximum memory that can be used by a cgroup and its descendants.
Try<Limit> maximum(const std::string& cgroup);

} // namespace memory {
} // namespace cgroups2

#endif // __CGROUPS_V2_HPP__
