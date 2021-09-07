# Abu Feed

This is part of the [Abu](http://github.com/FrancoisChabot/abu) meta-project.

## What are feeds?

1) Feeds are forward iterators.
2) Feeds distinguish being out of data and having reached the end of the data.
3) Feeds can be created from:
    - an input/sentinel iterator pair
    - a sequence of "chunks"

## Why feeds?

Feeds were implemented to support resumable parsers in 
[abu-parse](http://github.com/FrancoisChabot/abu-parse).

Specifically, they allows for code that can transparently ingest complete or 
partial ranges, which is what lets `abu-parse` offer both recursive descent and
state-machine based parsers from a single codebase.

## Reading from feeds

On the consumer side of things, feeds behave mostly like a bog-standard 
iterator. On top of that:
- It supports 2 different sentinel values:
    - `abu::feed::empty` means that the feed has currently run out of data, but *may* 
       be resumable eventually.
    - `abu::feed::end_of_feed` means that the feed has reached the true end of the 
       data.
- `abu::feed::checkpoint<T>` can be created from them, which can be used to rollback the feed.

You can use the `abu::Feed` and `abu::FeedOf<T>` concepts to constrain consumers.

Example:
```
void consumer(abu::FeedOf<int> auto& data) {
    auto checkpoint = data;

    while(data != abu::feed::empty) {
        std::cout << "reading " << *data << "\n";

        if(*data == 0) {
            data = checkpoint;
            throw std::runtime_error("we hit a 0");
        }

        ++data;
    }

    if(data == abu::feed::end_of_feed) {
        std::cout << "end of the feed reached!\n";
    }
}
```

In practice, `abu-feed` really shines when dealing with stateful and 
interuptible processes. Which would typically look like this:

```
template<abu::Feed FeedT>
struct some_process {
    using value_type = std::iter_value_type<FeedT>;
    
    FeedT& feed;

    bool resume() {
        while(feed != abu::feed::empty) {
            // ...
        }
    }
};
```

## Building feeds

### Adapted ranges

While `abu-feed`'s purpose is to handle interuptible streams, it's still 
convenient to allow directly adapting arbitrary input ranges. While this "could"
be done via streams (see below), the library provides adapters for all-at-once
ranges that let the compiler optimize things accordingly.

`auto adapt_range(iterator, sentinel);` will create a feed from any valid pair of
iterator and sentinel. If the iterator is not a forward range, appropriate 
buffering will be put into place.

```
int main() {
    std::vector<int> vec = {1, 2, 3, 4};

    // Will use iterators as checkpoints.
    auto vec_feed = abu::feed::adapt_range(vec.begin(), vec.end());
    consumer(vec_feed);

    // Will maintain a minimally required rollback buffer.
    auto cin_feed = abu::feed::adapt_range(
        std::istream_iterator<int>{std::cin}, 
        std::istream_iterator<int>{}
    );

    consumer(cin_feed);
}
```
### Streams

Streams present a series of "chunks" as a feed. The stream will take ownership
of the chunk and will destroy them once they can be guranteed to not be needed
anymore.

```
template<std::ranges::forward_range Chunk>
class stream {
public:
    void append(Chunk&& chunk, bool final=false);
    void finish();

    /* Feed interface */
};
```

Example:
```
std::vector<int> get_more_data();

int main() {
    abu::feed::stream<std::vector<int>> data;

    bool done;
    while(!done) {
        data.append(get_more_data());
        consumer(data);

        done = data == abu::feed::end_of_feed;
    }
}
```

A few notes on streams:
- Added chunks are let go as soon as no rollbacks to them is possible. If memory
  usage is a concern, consider adding smaller chunks more frequently.

## FAQ

### Why are feeds not forward ranges?

They used to be, but that required some unfortunate compromises. Feeds are meant
to go at the heart 

### What if I don't want a stream to detroy the data once it's done with it?

Use a proxy with shared (or without any) ownership of the underlying data. 

```
struct my_chunk {
    std::shared_ptr<std::vector> data;

    auto begin() const { return data->cbegin();}
    auto end() const { return data->cend();}
};
```

### What if I want a stream of input ranges?

No one has needed that yet.
