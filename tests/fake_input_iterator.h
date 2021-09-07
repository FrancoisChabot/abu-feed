//  Copyright 2017-2021 Francois Chabot
//  (francois.chabot.dev@gmail.com)
//
//  Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#ifndef ABU_FEED_TESTS_FAKE_INPUT_ITERATOR_H_INCLUDED
#define ABU_FEED_TESTS_FAKE_INPUT_ITERATOR_H_INCLUDED

#include <iterator>

namespace {

// This is used to test both forward_iterator and input_iterator behavior all
// at once.
template <std::forward_iterator T>
struct fake_input_iterator {
  using iterator_tag = std::input_iterator_tag;
  using difference_type = std::iter_difference_t<T>;
  using value_type = std::iter_value_t<T>;

  constexpr auto operator*() const { return *ite; }
  constexpr fake_input_iterator& operator++() {
    ++ite;
    return *this;
  }

  constexpr void operator++(int) { ite++; }
  constexpr bool operator==(const fake_input_iterator& rhs) const {
    return ite == rhs.ite;
  }
  T ite;
};

template<std::forward_iterator T>
auto as_input_iterator(T ite) {
  return fake_input_iterator<T>{ite};
}

static_assert(
    std::input_iterator<fake_input_iterator<std::vector<int>::iterator>>);
static_assert(
    !std::forward_iterator<fake_input_iterator<std::vector<int>::iterator>>);

}  // namespace

#endif