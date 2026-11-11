#pragma once

#include <string>

template <typename T, typename V>
class IDictionary{
public:
    virtual ~IDictionary() = default;
    virtual int GetCount() = 0;
    virtual int GetCapacity() const = 0;
    virtual V Get(K key) const = 0;
    virtual bool ContainsKey(K key) const = 0;
    virtual void Add(K key, V value) = 0;
    virtual void Remove(K key) = 0;
};