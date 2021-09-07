//  Copyright 2017-2021 Francois Chabot
//  (francois.chabot.dev@gmail.com)
//
//  Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#ifndef ABU_FEED_RANGE_ADAPTOR_H
#define ABU_FEED_RANGE_ADAPTOR_H

#include <deque>
#include <iterator>
#include <map>
#include <memory>

#include "abu/debug.h"
#include "abu/feed/tags.h"

namespace abu::feed {

namespace details_ {
template <std::input_iterator I, std::sentinel_for<I> S>
class input_range_adaptor {
  using value_type = std::iter_value_t<I>;

 public:
  input_range_adaptor(I begin, S end)
      : next_(std::move(begin)), end_(std::move(end)) {
    if (next_ != end_) {
      clients_.emplace(0, 1);
      buffer_.push_back(*next_);
      ++next_;
    }
  }

  void add_client(std::size_t pos) {
    // We should only ever add clients where another one already is.
    auto client = clients_.find(pos);
    abu_assume(client != clients_.end());
    abu_assume(client->second > 0);

    client->second += 1;
  }

  void remove_client(std::size_t pos) {
    auto client = clients_.find(pos);
    abu_assume(client != clients_.end());
    abu_assume(client->second > 0);

    client->second -= 1;
    if (client->second == 0) {
      clients_.erase(client);

      if (clients_.empty()) {
        // We are about to die anyways...
        return;
      }

      if (pos == buffer_start_) {
        auto next_start_abs = clients_.begin()->first;
        auto next_start_rel = next_start_abs - buffer_start_;
        buffer_.erase(buffer_.begin(),
                      std::next(buffer_.begin(), next_start_rel));
        buffer_start_ = next_start_abs;
      }
    }
  }

  decltype(auto) get(std::size_t pos) const {
    abu_assume(pos >= buffer_start_);
    abu_assume(pos - buffer_start_ < buffer_.size());
    auto rel_pos = pos - buffer_start_;

    return buffer_[rel_pos];
  }

  std::size_t advance(std::size_t pos) {
    abu_assume(pos >= buffer_start_);
    abu_assume(pos - buffer_start_ < buffer_.size());
    std::size_t rel_pos = pos - buffer_start_;

    // Append to the buffer if necessary
    if (rel_pos == buffer_.size() - 1) {
      if (next_ != end_) {
        buffer_.push_back(*next_);
        ++next_;
      }
    }

    // This is kinda bad...
    clients_[pos + 1] += 1;
    remove_client(pos);

    return pos + 1;
  }

  std::size_t is_end(std::size_t pos) const {
    abu_assume(pos >= buffer_start_);
    abu_assume(pos - buffer_start_ <= buffer_.size());
    auto rel_pos = pos - buffer_start_;

    return next_ == end_ && (rel_pos == buffer_.size());
  }

 private:
  // rel_pos -> count
  std::map<std::size_t, std::size_t> clients_;
  std::size_t buffer_start_ = 0;
  std::deque<value_type> buffer_;

  I next_;
  [[no_unique_address]] S end_;
};
}  // namespace details_

template <std::input_iterator I, std::sentinel_for<I> S>
class range_adaptor {
 public:
  using iterator_tag = std::forward_iterator_tag;
  using difference_type = std::iter_difference_t<I>;
  using value_type = std::iter_value_t<I>;

  range_adaptor() = default;
  range_adaptor(range_adaptor&&) = default;
  range_adaptor& operator=(range_adaptor&&) = default;

  ~range_adaptor() {
    if (impl_) {
      impl_->remove_client(pos_);
    }
  }

  range_adaptor(const range_adaptor& rhs) : pos_(rhs.pos_), impl_(rhs.impl_) {
    if (impl_) {
      impl_->add_client(pos_);
    }
  }

  range_adaptor& operator=(const range_adaptor& rhs) {
    if (rhs.impl_) {
      rhs.impl_->add_client(rhs.pos_);
    }

    if (impl_) {
      impl_->remove_client(pos_);
    }

    pos_ = rhs.pos_;
    impl_ = rhs.impl_;
    return *this;
  }

  range_adaptor(I begin, S end)
      : impl_(std::make_shared<details_::input_range_adaptor<I, S>>(
            std::move(begin), std::move(end))) {}

  decltype(auto) operator*() const { return impl_->get(pos_); }

  range_adaptor& operator++() {
    pos_ = impl_->advance(pos_);
    return *this;
  }

  range_adaptor operator++(int) {
    auto result = *this;
    pos_ = impl_->advance(pos_);
    return result;
  }

  bool operator==(const empty_feed_t&) const { return impl_->is_end(pos_); }
  constexpr bool operator==(const end_of_feed_t&) const {
    return impl_->is_end(pos_);
  }

  bool operator==(const range_adaptor& rhs) const {
    return pos_ == rhs.pos_ && impl_ == rhs.impl_;
  }

 private:
  std::size_t pos_ = 0;
  std::shared_ptr<details_::input_range_adaptor<I, S>> impl_;
};

template <std::forward_iterator I, std::sentinel_for<I> S>
class range_adaptor<I, S> {
 public:
  using iterator_tag = std::forward_iterator_tag;
  using difference_type = std::iter_difference_t<I>;
  using value_type = std::iter_value_t<I>;

  constexpr range_adaptor() = default;
  constexpr range_adaptor(I begin, S end)
      : next_(std::move(begin)), end_(std::move(end)) {}

  constexpr decltype(auto) operator*() const { return *next_; }
  constexpr range_adaptor& operator++() {
    ++next_;
    return *this;
  }
  constexpr range_adaptor operator++(int) {
    auto result = *this;
    ++next_;
    return result;
  }
  constexpr bool operator==(const empty_feed_t&) const {
    return next_ == end_;
  }
  constexpr bool operator==(const end_of_feed_t&) const {
    return next_ == end_;
  }

  constexpr bool operator==(const range_adaptor& rhs) const {
    return next_ == rhs.next_ && end_ == rhs.end_;
  }

 private:
  I next_;
  [[no_unique_address]] S end_;
};

}  // namespace abu::feed

#endif