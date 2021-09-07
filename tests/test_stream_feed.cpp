// Copyright 2021 Francois Chabot

// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0

// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <vector>

#include "abu/feed.h"
#include "gtest/gtest.h"

TEST(stream_feed, peek_and_read) {
  abu::feed::stream<std::vector<int>> stream;

  auto i = stream.begin();
  auto e = stream.end();

  EXPECT_NE(i, e);
  EXPECT_EQ(i, abu::feed::empty);

  stream.append({1, 2, 3});

  EXPECT_NE(i, abu::feed::empty);

  EXPECT_EQ(*i++, 1);
  EXPECT_EQ(*i++, 2);
  EXPECT_EQ(*i++, 3);
  EXPECT_EQ(i, abu::feed::empty);
  EXPECT_NE(i, e);
}
