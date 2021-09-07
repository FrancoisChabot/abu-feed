//  Copyright 2017-2021 Francois Chabot
//  (francois.chabot.dev@gmail.com)
//
//  Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#ifndef ABU_FEED_STREAM_H
#define ABU_FEED_STREAM_H

#include "abu/debug.h"
#include "abu/feed/tags.h"

namespace abu::feed {

template <typename T>
concept Chunk = std::ranges::forward_range<T> && std::default_initializable<T>;

namespace details_ {

  template <Chunk ChunkT>
  struct stream_node {
    stream_node() = default;
    stream_node(ChunkT&& in_data) : data(std::move(in_data)) {}

    ChunkT data;
    std::shared_ptr<stream_node<ChunkT>> next;
  };
}  // namespace details_

template <Chunk ChunkT>
class stream_feed;

template <Chunk ChunkT>
class stream {
 public:
  using chunk_type = ChunkT;
  stream() : tail_(std::make_shared<details_::stream_node<chunk_type>>()) {}

  void append(chunk_type&& chunk) {
    auto new_node =
        std::make_shared<details_::stream_node<chunk_type>>(std::move(chunk));
    if (tail_) {
      tail_->next = new_node;
    }
    tail_ = new_node;
  }

  void finish() { tail_.reset(); }

  inline stream_feed<ChunkT> begin();
  end_of_feed_t end() const { return end_of_feed; }

 private:
  std::shared_ptr<details_::stream_node<ChunkT>> tail_;
};

template <Chunk ChunkT>
class stream_feed {
 public:
  using iterator_tag = std::forward_iterator_tag;
  using difference_type = std::ranges::range_difference_t<ChunkT>;
  using value_type = std::ranges::range_value_t<ChunkT>;

  std::shared_ptr<details_::stream_node<ChunkT>> node_;
  std::ranges::iterator_t<ChunkT> next_;

  stream_feed() = default;
  explicit stream_feed(std::shared_ptr<details_::stream_node<ChunkT>> node)
      : node_(std::move(node)), next_(std::ranges::begin(node_->data)) {}

  decltype(auto) operator*() const {
    if (next_ == std::ranges::end(node_->data) && node_->next) {
      return *std::ranges::begin(node_->next->data);
    }
    return *next_;
  }

  stream_feed& operator++() {
    if (next_ == std::ranges::end(node_->data) && node_->next) {
      node_ = node_->next;
      next_ = std::ranges::begin(node_->data);
    }

    ++next_;
    return *this;
  }

  stream_feed operator++(int) {
    auto result = *this;
    ++(*this);
    return result;
  }

  bool operator==(const stream_feed& rhs) const = default;

  bool operator==(const empty_feed_t&) const {
    return !node_ || (!node_->next && next_ == std::ranges::end(node_->data));
  }

  bool operator==(const end_of_feed_t&) const { return !node_; }
};

template <Chunk ChunkT>
stream_feed<ChunkT> stream<ChunkT>::begin() {
  static_assert(std::forward_iterator<stream_feed<ChunkT>>);
  return stream_feed<ChunkT>{tail_};
}
/*
namespace abu {

struct data_feed_chunk_elem {
  virtual ~data_feed_chunk_elem() = default;
  std::shared_ptr<data_feed_chunk_elem> next_chunk;
};

template <typename ChunkData>
struct data_feed_chunk : public data_feed_chunk_elem {
  explicit data_feed_chunk(ChunkData init_data) : data(std::move(init_data)) {}

  ChunkData data;
};

// ***** basic_data_feed *****
template <std::ranges::input_range ChunkData>
class basic_data_feed {
  using chunk_type = data_feed_chunk<ChunkData>;
  using data_iterator_type = std::ranges::iterator_t<ChunkData>;
  using data_sentinel_type = std::ranges::sentinel_t<ChunkData>;

 public:
  using token_type = std::ranges::range_value_t<ChunkData>;

  basic_data_feed()
      : last_chunk_(std::make_shared<data_feed_chunk_elem>()),
        front_sentinel_(last_chunk_) {}

  constexpr void add(ChunkData chunk) {
    auto chunk_ptr =
        std::make_shared<data_feed_chunk<ChunkData>>(std::move(chunk));

    last_chunk_->next_chunk = chunk_ptr;

    if (!current_chunk_) {
      current_chunk_ = chunk_ptr;
      current_chunk_next_ = std::ranges::begin(current_chunk_->data);
      current_chunk_end_ = std::ranges::end(current_chunk_->data);
    }

    last_chunk_ = std::move(chunk_ptr);
  }

  constexpr const token_type& peek() const {
    abu_precondition(!empty());

    return *current_chunk_next_;
  }

  constexpr bool empty() const { return !current_chunk_; }

  constexpr token_type read() {
    abu_assume(!empty());

    auto result = *current_chunk_next_++;
    if (current_chunk_next_ == current_chunk_end_) {
      // Just in case iterators require to be outlived by their container
      current_chunk_next_ = {};
      current_chunk_end_ = {};

      // This is intentionally not be a move!
      // A checkpoint could be holding on to the current_chunk_.
      current_chunk_ =
          std::static_pointer_cast<chunk_type>(current_chunk_->next_chunk);

      if (current_chunk_) {
        current_chunk_next_ = std::ranges::begin(current_chunk_->data);
        current_chunk_end_ = std::ranges::end(current_chunk_->data);
      }
    }
    return result;
  }

 protected:
  std::shared_ptr<chunk_type> current_chunk_;
  std::shared_ptr<data_feed_chunk_elem> last_chunk_;

  data_iterator_type current_chunk_next_;
  data_sentinel_type current_chunk_end_;

  std::weak_ptr<data_feed_chunk_elem> front_sentinel_;
};

// ***** Input feed *****
template <std::ranges::input_range ChunkData>
struct feed_input_checkpoint {
  std::shared_ptr<data_feed_chunk_elem> chunk;
};

template <std::ranges::input_range ChunkData>
struct data_feed : public basic_data_feed<ChunkData> {
  using token_type = std::ranges::range_value_t<ChunkData>;

 private:
  std::deque<token_type> rollback_buffer_;
};

// ***** Forward feed *****
template <std::ranges::forward_range ChunkData>
struct feed_forward_checkpoint {
  using pointer_type = std::shared_ptr<data_feed_chunk_elem>;
  using iterator_type = std::ranges::iterator_t<ChunkData>;

  pointer_type chunk;
  iterator_type next;
};

template <std::ranges::forward_range ChunkData>
struct data_feed<ChunkData> : public basic_data_feed<ChunkData> {
  using checkpoint_type = feed_forward_checkpoint<ChunkData>;
  using chunk_type = data_feed_chunk<ChunkData>;

  constexpr checkpoint_type make_checkpoint() {
    if (!this->current_chunk_) {
      return checkpoint_type{this->last_chunk_, {}};
    }
    return checkpoint_type{this->current_chunk_, this->current_chunk_next_};
  }

  constexpr void rollback(checkpoint_type cp) {
    if (cp.chunk == this->front_sentinel_.lock()) {
      this->current_chunk_ =
          std::static_pointer_cast<chunk_type>(cp.chunk->next_chunk);

      this->current_chunk_next_ =
          std::ranges::begin(this->current_chunk_->data);
      this->current_chunk_end_ = std::ranges::end(this->current_chunk_->data);
    }

    else {
      this->current_chunk_ = std::static_pointer_cast<chunk_type>(cp.chunk);
      this->current_chunk_next_ = std::move(cp.next);
      this->current_chunk_end_ = std::ranges::end(this->current_chunk_->data);
    }
  }
};
*/
}  // namespace abu::feed

#endif