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

#include <set>
#include <string>

#include <stout/tests/utils.hpp>
#include <stout/try.hpp>

#include "linux/cgroups2.hpp"

using std::set;
using std::string;

namespace mesos {
namespace internal {
namespace tests {

class Cgroups2Test : public TemporaryDirectoryTest {};


class Cgroups2MemoryTest : public Cgroups2Test
{
public:
  static void SetUpTestCase()
  {
    Cgroups2Test::SetUpTestCase();
    cgroups2::prepare({"memory"});

    // Clean up from previous runs.
    cgroups2::destroy("test-cgroup");

    EXPECT_SOME(cgroups2::create("test-cgroup"));
    EXPECT_SOME(cgroups2::subsystems::enable("test-cgroup", {"memory"}));
    EXPECT_SOME_TRUE(cgroups2::subsystems::enabled("test-cgroup", {"memory"}));
  }


  static void TearDownTestCase()
  {
    cgroups2::destroy("test-cgroup");
    Cgroups2Test::TearDownTestCase();
  }

  const string cgroup = "test-cgroup";
};


TEST_F(Cgroups2Test, ROOT_CGROUPS2_Enabled)
{
  EXPECT_TRUE(cgroups2::enabled());
}


TEST_F(Cgroups2Test, ROOT_CGROUPS2_AvailableSubsystems)
{
  Try<set<string>> available = cgroups2::subsystems::available(
    cgroups2::ROOT_CGROUP);

  ASSERT_SOME(available);
  EXPECT_TRUE(available->count("cpu") == 1);
}

TEST_F(Cgroups2Test, ROOT_CGROUPS2_Prepare)
{
  EXPECT_SOME(cgroups2::prepare({"cpu"}));
  Try<set<string>> available =
    cgroups2::subsystems::available(cgroups2::ROOT_CGROUP);
  EXPECT_SOME(available);
  EXPECT_TRUE(available.get().count("cpu") == 1);
  EXPECT_SOME_TRUE(
    cgroups2::subsystems::enabled(cgroups2::ROOT_CGROUP, {"cpu"}));
}


TEST_F(Cgroups2MemoryTest, ROOT_CGROUPS2_MemoryUsage)
{
  // Does not exist for the root cgroup.
  EXPECT_ERROR(cgroups2::memory::usage(cgroups2::ROOT_CGROUP));

  EXPECT_SOME(cgroups2::memory::usage(cgroup));
}

} // namespace tests {
} // namespace internal {
} // namespace mesos {

