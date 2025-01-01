#pragma once

#include <optional>
#include <unordered_map>

#include "symbol_data.hpp"

class SymbolRegistry {
public:
    using SymbolID = size_t;

    SymbolID AddSymbol(const SymbolData& data);
    [[nodiscard]] SymbolID GetSymbol(const std::string_view& name) const ;
    std::optional<const SymbolData> GetSymbolData(SymbolID id);
    std::optional<const SymbolData> GetSymbolData(const std::string_view& name);

    const std::vector<SymbolID>& GetAlphabet() { return _alphabet; }
    const std::vector<SymbolID>& GetNonTerminals() { return _nonTerminals; }

private:
    std::unordered_map<SymbolID, SymbolData> registry{};

    std::vector<SymbolID> _alphabet{};
    std::vector<SymbolID> _nonTerminals{};
};