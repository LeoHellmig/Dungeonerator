//
// Created by leohe on 01/01/2025.
//
#include <iostream>

#include "symbol_registry.hpp"

SymbolRegistry::SymbolID SymbolRegistry::AddSymbol(const SymbolData &data) {
    const auto id = std::hash<std::string>{}(data.name);
    if (registry.contains(id)) {
        std::cout << "Tried adding multiple symbols with the same name" << data.name.data() << std::endl;
    }
    registry[id] = data;

    if (data.terminal) {
        _alphabet.emplace_back(id);
    }
    else {
        _nonTerminals.emplace_back(id);
    }

    return id;
}

SymbolRegistry::SymbolID SymbolRegistry::GetSymbol(const std::string_view &name) const {
    const auto id = std::hash<std::string>{}(name.data());
    if (!registry.contains(id)) {
        std::cout << "Requested non-existing symbol: " << name.data() << std::endl;

        return 0;
    }
    return id;
}

std::optional<const SymbolData> SymbolRegistry::GetSymbolData(const SymbolID id) {
    if (!registry.contains(id)) {
        std::cout << "Tried requesting symbol data from non existing symbol: " << id << std::endl;
        return std::nullopt;
    }
    return registry[id];
}

std::optional<const SymbolData> SymbolRegistry::GetSymbolData(const std::string_view &name) {
    const auto id = std::hash<std::string>{}(name.data());
    if (!registry.contains(id)) {
        std::cout << "Tried to request symbol data from non existing symbol: " << name.data() << std::endl;
        return std::nullopt;
    }
    return registry[id];
}
