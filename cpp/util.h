#pragma once

#include <deque>
#include <list>
#include <mutex>
#include <shared_mutex>
#include <vector>


template<class T>
class PartitionedList {
public:
  using Partition = std::deque<T>;

  PartitionedList(size_t num_partitions) : partitions_(num_partitions) { }

  void push_back(size_t part_id, const T& value) {
    partitions_[part_id].push_back(value);
  }

  class Iterator {
  public:
    Iterator(std::vector<Partition>& partitions, bool is_begin) : partitions_(partitions) {
      if (is_begin) {
        part_id_ = 0;
        SkipEmptyPartitions();
        iter_ = partitions_[part_id_].begin();
      } else {
        part_id_ = partitions_.size() - 1;
        iter_ = partitions_[part_id_].end();
      }
    }

    T& operator*() { return *iter_; }

    bool operator!=(const Iterator& other) const {
      return part_id_ != other.part_id_ || iter_ != other.iter_;
    }

    Iterator& operator++() {
      if (++iter_ == partitions_[part_id_].end()) {
        if (part_id_ >= partitions_.size() - 1) return *this;
        ++part_id_;
        SkipEmptyPartitions();
        iter_ = partitions_[part_id_].begin();
      }
      return *this;
    }

  private:
    void SkipEmptyPartitions() {
      while (partitions_[part_id_].empty()) {
        if (part_id_ >= partitions_.size() - 1) return;
        ++part_id_;
      }
    }

    std::vector<Partition>& partitions_;
    size_t part_id_;
    typename std::deque<T>::iterator iter_;
  };

  Iterator begin() { return Iterator(partitions_, true); }

  Iterator end() { return Iterator(partitions_, false); }

  size_t size() const {
    size_t total = 0;
    for (const Partition& p : partitions_) total += p.size();
    return total;
  }

private:
  std::vector<Partition> partitions_;
};


class ConcurrentIntegerSet {
public:
  ConcurrentIntegerSet() : size_(0), buckets_size_(0), buckets_(nullptr) { Resize(); }
  ~ConcurrentIntegerSet() { delete[] buckets_; }

  inline bool InsertIfNotExist(int64_t value) {
    {
      std::shared_lock<std::shared_mutex> lock(mutex_);
      auto& bucket = buckets_[value & (buckets_size_ - 1)];
      for (auto& v : bucket) {
        if (v == value) return false;
      }
    }
    std::unique_lock<std::shared_mutex> lock(mutex_);
    auto& bucket = buckets_[value & (buckets_size_ - 1)];
    bucket.push_back(value);
    ++size_;
    if (buckets_size_ * 1.5 < size_) Resize();
    return true;
  }

private:
  size_t size_;
  size_t buckets_size_;
  std::vector<int64_t>* buckets_;
  std::shared_mutex mutex_;

  void Resize() {
    size_t new_size = 16;
    while (new_size < size_ * 3) {
      new_size *= 2;
    }
    auto new_buckets = new std::vector<int64_t>[new_size];
    for (size_t i = 0; i < buckets_size_; ++i) {
      auto& bucket = buckets_[i];
      for (auto& value : bucket) {
        new_buckets[value & (new_size - 1)].push_back(value);
      }
    }
    delete[] buckets_;
    buckets_ = new_buckets;
    buckets_size_ = new_size;
  }
};


template<size_t NumBuckets>
class ConcurrentNumericSet {
public:
  ConcurrentNumericSet(double precision) : precision_(precision) { }

  inline bool InsertIfNotExist(int64_t value) {
    return ints_[value % NumBuckets].InsertIfNotExist(value);
  }

  inline bool InsertIfNotExist(double value) {
    if (precision_ * INT64_MAX > value) {
      int64_t value_as_int = static_cast<int64_t>(value / precision_);
      return double_as_ints_[value_as_int % NumBuckets].InsertIfNotExist(value_as_int);
    } else {
      double offset_value = value - precision_ * INT64_MAX;
      int64_t value_as_int = static_cast<int64_t>(offset_value / precision_);
      return big_doubles_[value_as_int % NumBuckets].InsertIfNotExist(value_as_int);
    }
  }

private:
  const double precision_;

  ConcurrentIntegerSet ints_[NumBuckets];
  ConcurrentIntegerSet double_as_ints_[NumBuckets];
  ConcurrentIntegerSet big_doubles_[NumBuckets];
};


template<size_t ChunkSize = 4 * 1024>
class ObjectPool {
private:
  struct Chunk {
    char data[ChunkSize];
    size_t next = 0;
  };

  inline Chunk& GetFreeChunk(size_t require) {
    if (chunks_.back().next + require > ChunkSize) {
      chunks_.emplace_back();
    }
    return chunks_.back();
  }

  std::list<Chunk> chunks_;
  size_t uncommitted_size_;

public:
  ObjectPool() : uncommitted_size_(0) { chunks_.emplace_back(); }

  ObjectPool(const ObjectPool&) = delete;
  ObjectPool& operator=(const ObjectPool&) = delete;

  template <class T, class... Valty>
  T* EmplaceObject(Valty&&... val) {
    static_assert(sizeof(T) <= ChunkSize, "ChunkSize too small");
    uncommitted_size_ = sizeof(T);
    Chunk& chunk = GetFreeChunk(uncommitted_size_);
    return new (chunk.data + chunk.next) T(std::forward<Valty>(val)...);
  }

  inline void CommitLastObject() {
    chunks_.back().next += uncommitted_size_;
    uncommitted_size_ = 0;
  }
};

