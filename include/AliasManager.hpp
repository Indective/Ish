#pragma once
#include "Types.hpp"

#include <iostream>
#include <unordered_map>
#include <vector>
#include <optional>

enum class AliasTokenType
{
    Word,
    EqualSign,
    Value
};

struct AliasToken
{
    AliasTokenType type;
    std::string value;
};

struct Alias
{
    std::string name;
    std::string value;
};

namespace AliasManager
{
    static constexpr const char* filename = "/.ish_aliases";

    void load_aliases();
    void replace_alias(Command& cmd);
    void reload_aliases(const std::string& path);

    std::optional<std::vector<AliasToken>> tokenize_alias(const std::string &line);
    std::optional<Alias> parse_alias(const std::vector<AliasToken> &tokens);
}