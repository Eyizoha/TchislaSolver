﻿#pragma once

#include <deque>
#include <list>
#include <mutex>
#include <shared_mutex>
#include <unordered_set>
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


template <typename T, size_t NumBuckets = 10>
class ConcurrentSet {
public:
  struct Bucket {
    std::unordered_set<T> data;
    mutable std::shared_mutex mutex;
  };

  ConcurrentSet() : buckets(NumBuckets) {}

  inline bool InsertIfNotExist(const T& key) {
    size_t bucket_index = std::hash<T>{}(key) % buckets.size();
    Bucket& bucket = buckets[bucket_index];

    bool exist = false;
    {
      std::shared_lock<std::shared_mutex> lock(bucket.mutex);
      exist = bucket.data.find(key) != bucket.data.end();
    }
    if (exist) return false;

    std::unique_lock<std::shared_mutex> lock(bucket.mutex);
    bucket.data.insert(key);
    return true;
  }

private:
  std::vector<Bucket> buckets;
};


template <size_t NumBuckets = 10>
class ConcurrentNumericSet {
public:
  ConcurrentNumericSet(double precision) : precision_(precision) { }

  inline bool InsertIfNotExist(int64_t value) {
    return ints_.InsertIfNotExist(value);
  }

  inline bool InsertIfNotExist(double value) {
    if (precision_ * INT64_MAX > value) {
      int64_t value_as_int = static_cast<int64_t>(value / precision_);
      return double_as_ints_.InsertIfNotExist(value_as_int);
    } else {
      return oringin_doubles_.InsertIfNotExist(value);
    }
  }

private:
  const double precision_;

  ConcurrentSet<int64_t, NumBuckets> ints_;
  ConcurrentSet<int64_t, NumBuckets> double_as_ints_;
  ConcurrentSet<double, NumBuckets> oringin_doubles_;
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

