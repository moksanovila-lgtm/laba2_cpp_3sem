#pragma once

#include <string>
#include "Person.hpp"
#include "ArraySequence.hpp"

class CvsLoader{
public:
    static ArraySequence<Person> Load(const std::string& path);
    static void Save(const std::string& path, const ArraySequence<Person>& data);
};