#pragma once

#include <optional>
#include <random>
#include <set>

class Grammar {
public:
    struct Symbol {
        bool terminal = false;
        std::string name;

        bool operator==(const Symbol & a) const {
            return this->terminal == a.terminal
                    && this->name == a.name;
        }
    };

    struct Rule {
        std::vector<Symbol> lhs;
        std::vector<Symbol> rhs;
    };

    std::optional<std::list<Symbol>> ApplyRules(const Symbol& start) {
        mString.clear();
        mString.emplace_back(start);

        // restrict depth to 1000
        uint32_t depth = 0;
        std::random_device rd;
        std::mt19937 mt(rd());
        std::uniform_int_distribution<uint32_t> dist(mRules.size());
        std::set<size_t> badRules = {};

        while (depth < 1000 && badRules.size() !=  mRules.size()) {
            auto idx = dist(mt) % mRules.size();
            const Rule& rule = mRules[idx];
            if (rule.lhs.empty()) {
                continue;
            }

            auto subRange = std::ranges::search(mString.begin(), mString.end(), rule.lhs.begin(), rule.lhs.end());
            if (subRange.empty()) {
                std::cout << "Sub range is empty" << std::endl;
                badRules.insert(idx);
                continue;
            }

            badRules.clear();
            // inserting symbols from rule
            mString.insert(subRange.begin(), rule.rhs.begin(), rule.rhs.end());
            // erase old subrange
            mString.erase(subRange.begin(), subRange.end());
            ++depth;
        }

        return mString;
    }

    void SetRules(const std::vector<Rule>& rules) {
        mRules = rules;
    }

    void SetAlphabet(const std::vector<Symbol>& alphabet) {
        mAlphabet.resize(alphabet.size());
        for (size_t i = 0; i < alphabet.size(); ++i) {
            mAlphabet[i] = alphabet[i];
            mAlphabet[i].terminal = true;
        }
    }

    std::vector<Symbol> mAlphabet {};
    std::list<Symbol> mString = {};
    std::vector<Rule> mRules = {};
};

// Somehow make an object that can chain multiple grammars in a row
class Dungeonerator {
public:





private:



};
