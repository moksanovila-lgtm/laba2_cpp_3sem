#pragma once

#include <functional>
#include <list>
#include <vector>
#include <stdexcept>
#include <algorithm>
#include "IIndex.h"

using std::size_t;

template <typename K, typename V>
class HashIndex : public IIndex<K, V> {
public:
    using HashFunc = std::function<std::size_t(const K&)>;
private:
    using Bucket = std::list<std::pair<K, V>>;

    std::vector<Bucket> buckets;
    HashFunc hash;
    size_t p;
    size_t q;
    size_t collisions = 0;

    size_t IndexFor(const K& key) const {return hash(key) % buckets.size();}
    void Rehash(size_t newCapacity);
    void MaybeShrink();
    void MaybeGrow();


public:
    HashIndex(HashFunc hash, size_t capacity = 16, size_t p = 2, size_t q = 2);

    void Add(const K& key, const V& value) override;
    bool ContainsKey(const K& key) const override;
    V Get(const K& key) const override;
    size_t Size() const override;
    const char* Name() const override {return "hash_chain";}

    size_t Capacity() const {return buckets.size();}
    size_t Collisions() const {return collisions;}
    size_t MaxChainLength() const;

};


template <typename K, typename V>
HashIndex<K, V>::HashIndex(HashFunc hash, size_t capacity, size_t p, size_t q) : 
    buckets(capacity), hash(std::move(hash)), p(p), q(q) {
    if(capacity == 0){
        throw std::invalid_argument("HashIndex: capacity must be > 0");
    }
    if(q <= 1 || p < q){
        throw std::invalid_argument("HashIndex: require p >= q > 1");
    }
    if(!this->hash){
        throw std::invalid_argument("HashIndex: hash function is empty");
    }
}

template <typename K, typename V>
void HashIndex<K, V>::Add(const K& key, const V& value){
    size_t idx = IndexFor(key);
    Bucket& bucket = buckets[idx];
    if(!buckets.empty()){
        collisions++;
    }
    typename::iterator it = bucket.begin();
    while(it != bucket.end()){
        if(it->first == key){
            if constexpr (requires(V& a, const V& b) {
                a.insert(a.end(), b.begin(), b.end());
            }) {
                it->second.insert(it->second.end(), value.begin(), value.end());
            } else {
                it->second = value;
            }
            return;
        }
        it++;
    }
    bucket.push_back(std::make_pair(key, value));
    size++;
    MaybeGrow();
}


template <typename K, typename V>
bool HashIndex<K, V>::ContainsKey(const K& key) const {
    size_t idx = IndexFor(key);
    const Bucket& bucket = buckets[idx];
    typename Bucket::const_iterator it = bucket.begin();
    while (it != bucket.end()) {
        if (it->first == key) return true;
        it++;
    }
    return false;
}

template <typename K, typename V>
V HashIndex<K, V>::Get(const K& key) const {
    size_t idx = IndexFor(key);
    const Bucket& bucket = buckets[idx];

    typename Bucket::const_iterator it = bucket.begin();
    while (it != bucket.end()) {
        if (it->first == key) return it->second;
        it++;
    }
    throw std::out_of_range("HashIndex::Get: key not found");
}

template <typename K, typename V>
size_t HashIndex<K, V>::MaxChainLength() const {
    size_t maxLen = 0;
    for (size_t i = 0; i < buckets.size(); ++i) {
        if (buckets[i].size() > maxLen) {
            maxLen = buckets[i].size();
        }
    }
    return maxLen;
}

template <typename K, typename V>
void HashIndex<K, V>::MaybeGrow() {
    if (size >= buckets.size()) {
        Rehash(buckets.size() * q);
    }
}

template <typename K, typename V>
void HashIndex<K, V>::MaybeShrink() {
    constexpr size_t minCapacity = 16;
    if (buckets.size() > minCapacity && size <= buckets.size() / p) {
        size_t newCap = buckets.size() / q;
        if (newCap < minCapacity) newCap = minCapacity;
        if (newCap < buckets.size()) {
            Rehash(newCap);
        }
    }
}

template <typename K, typename V>
void HashIndex<K, V>::Rehash(size_t newCapacity) {
    if (newCapacity == 0) {
        throw std::invalid_argument("HashIndex::Rehash: newCapacity must be > 0");
    }
    std::vector<Bucket> newBuckets(newCapacity);
    typename std::vector<Bucket>::iterator oldBucketIt = buckets.begin();
    while (oldBucketIt != buckets.end()) {
        Bucket& oldBucket = *oldBucketIt;
        typename Bucket::iterator pairIt = oldBucket.begin();
        while (pairIt != oldBucket.end()) {
            size_t newIdx = hash(pairIt->first) % newCapacity;
            newBuckets[newIdx].push_back(std::make_pair(pairIt->first, pairIt->second));
            pairIt++;
        }
        oldBucketIt++;
    }
    buckets = std::move(newBuckets);
    collisions = 0;
}
