#include "grammar.hpp"
#include <random>
#include <set>
#include <iostream>

void Grammar::PrintInfo() const {
    std::cout << "---------- \nGrammar Info" << std::endl;
    std::cout << "Alphabet: ";
    for (const auto & symbol : _registry.GetAlphabet()) {
        std::cout << _registry.GetSymbolData(symbol)->name;
    }
    std::endl(std::cout);
    std::cout << "Non Terminals: ";
    for (const auto & symbol : _registry.GetNonTerminals()) {
        std::cout << _registry.GetSymbolData(symbol)->name;
    }
    std::endl(std::cout);

    std::cout << "Starting String: ";
    for (const auto & symbol : _startString) {
        std::cout << _registry.GetSymbolData(symbol)->name;
    }
    std::endl(std::cout);

    std::cout << "Rules: " << std::endl;

    int ruleCounter = 0;
    for (const auto& rule : _rules) {
        std::cout << "Rule " << ruleCounter << std::endl;
        std::cout << "  lhs: ";
        for (const auto& symbol : rule.lhs) {
            std::cout << _registry.GetSymbolData(symbol)->name;
        }
        std::endl(std::cout);
        std::cout << "  rhs: ";
        for (const auto& symbol : rule.rhs) {
            std::cout << _registry.GetSymbolData(symbol)->name;
        }
        std::endl(std::cout);
        ruleCounter++;
    }
}

void Grammar::ExecuteGrammar() {
    // restrict depth to 1000
    uint32_t depth = 0;
    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_int_distribution<uint32_t> dist(_rules.size());
    std::set<size_t> badRules = {};

    _outputString = std::list<SymbolID>(_startString.begin(), _startString.end());

    while (depth < 1000 && badRules.size() !=  _rules.size()) {
        auto idx = dist(mt) % _rules.size();
        const RuleInternal& rule = _rules[idx];
        if (rule.lhs.empty() || badRules.contains(idx)) {
            continue;
        }

        auto subRange = std::ranges::search(_outputString.begin(), _outputString.end(), rule.lhs.begin(), rule.lhs.end());
        if (subRange.empty()) {
            //std::cout << "Sub range is empty" << std::endl;
            badRules.insert(idx);
            continue;
        }

        // String has been updated, other rules might work again
        badRules.clear();
        // inserting symbols from rule
        _outputString.insert(subRange.begin(), rule.rhs.begin(), rule.rhs.end());
        // erase old subrange
        _outputString.erase(subRange.begin(), subRange.end());
        ++depth;
    }
}

void Grammar::ConvertRules(const std::vector<Rule> &rules) {
    for (const auto& rule : rules) {
        RuleInternal internal;

        for (const auto& symbol : rule.lhs) {
            internal.lhs.emplace_back(_registry.GetSymbol(symbol));
        }
        for (const auto& symbol : rule.rhs) {
            internal.rhs.emplace_back(_registry.GetSymbol(symbol));
        }

        _rules.emplace_back(internal);
    }
}
