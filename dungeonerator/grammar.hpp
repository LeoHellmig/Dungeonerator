#pragma once

#include "symbol_registry.hpp"
#include "grammar_rule.hpp"

class Grammar {
public:
    using SymbolID = SymbolRegistry::SymbolID;

    explicit Grammar(SymbolRegistry& registry, const std::vector<Rule>& rules)
    : _registry(registry) {
        ConvertRules(rules);
    }

    void PrintInfo() const;

    void ExecuteGrammar(const std::list<SymbolID>& startString);

    const std::list<SymbolID>& GetString() {
        return _outputString;
    }

private:
    struct RuleInternal {
        std::vector<SymbolID> lhs {};
        std::vector<SymbolID> rhs {};
    };

    void ConvertRules(const std::vector<Rule>& rules);

    SymbolRegistry& _registry;
    std::list<SymbolID> _outputString {};
    std::vector<RuleInternal> _rules {};
};