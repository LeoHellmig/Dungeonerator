#pragma once

#include <vector>
#include <string>

// The object that the user creates for making new rules
struct Rule {
    std::vector<std::string> lhs;
    std::vector<std::string> rhs;
};