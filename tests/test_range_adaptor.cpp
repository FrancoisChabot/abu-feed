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
#include "fake_input_iterator.h"
#include "gtest/gtest.h"

TEST(adapted_forward_range, peek_and_read) {
  std::vector<int> raw_data = {1, 2, 3, 4};

  auto sut = abu::feed::adapt_range(
      std::begin(raw_data), std::end(raw_data));

  EXPECT_NE(sut, abu::feed::empty);
  EXPECT_EQ(*sut, 1);

  EXPECT_NE(sut, abu::feed::empty);
  EXPECT_EQ(*++sut, 2);

  EXPECT_NE(sut, abu::feed::empty);
  EXPECT_EQ(*++sut, 3);

  EXPECT_NE(sut, abu::feed::empty);
  EXPECT_EQ(*++sut, 4);

  EXPECT_EQ(++sut, abu::feed::empty);
}

TEST(adapted_forward_range, rollback) {
  std::vector<int> raw_data = {1, 2, 3, 4};

  auto sut = abu::feed::adapt_range(
      std::begin(raw_data), std::end(raw_data));

  auto start = sut;
  EXPECT_EQ(*sut, 1);
  EXPECT_EQ(*++sut, 2);

  auto cp = sut;
  EXPECT_EQ(*++sut, 3);
  EXPECT_EQ(*++sut, 4);
  EXPECT_EQ(++sut, abu::feed::empty);

  sut = cp;

  EXPECT_NE(sut, abu::feed::empty);
  EXPECT_EQ(*++sut, 3);
  EXPECT_EQ(*++sut, 4);
  EXPECT_EQ(++sut, abu::feed::empty);
  sut = start;

  EXPECT_EQ(*sut, 1);
  EXPECT_EQ(*++sut, 2);
  EXPECT_EQ(*++sut, 3);
  EXPECT_EQ(*++sut, 4);
  EXPECT_EQ(++sut, abu::feed::empty);
}

TEST(adapted_input_range, peek_and_read) {
  std::vector<int> raw_data = {1, 2, 3, 4};

  auto sut = abu::feed::adapt_range(as_input_iterator(std::begin(raw_data)),
                                    as_input_iterator(std::end(raw_data)));

  EXPECT_NE(sut, abu::feed::empty);
  EXPECT_EQ(*sut, 1);

  EXPECT_NE(sut, abu::feed::empty);
  EXPECT_EQ(*++sut, 2);

  EXPECT_NE(sut, abu::feed::empty);
  EXPECT_EQ(*++sut, 3);

  EXPECT_NE(sut, abu::feed::empty);
  EXPECT_EQ(*++sut, 4);

  EXPECT_EQ(++sut, abu::feed::empty);
}

TEST(adapted_input_range, rollback) {
  std::vector<int> raw_data = {1, 2, 3, 4};

  auto sut = abu::feed::adapt_range(as_input_iterator(std::begin(raw_data)),
                                    as_input_iterator(std::end(raw_data)));

  auto pre_start = sut;
  sut = pre_start;

  auto start = sut;
  EXPECT_EQ(*sut, 1);
  EXPECT_EQ(*++sut, 2);

  auto cp = sut;
  EXPECT_EQ(*++sut, 3);
  EXPECT_EQ(*++sut, 4);
  EXPECT_EQ(++sut, abu::feed::empty);

  sut = cp;

  EXPECT_NE(sut, abu::feed::empty);
  EXPECT_EQ(*++sut, 3);
  EXPECT_EQ(*++sut, 4);
  EXPECT_EQ(++sut, abu::feed::empty);
  sut = start;

  EXPECT_EQ(*sut, 1);
  EXPECT_EQ(*++sut, 2);
  EXPECT_EQ(*++sut, 3);
  EXPECT_EQ(*++sut, 4);
  EXPECT_EQ(++sut, abu::feed::empty);
}
