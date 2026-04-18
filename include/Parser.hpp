#pragma once
#include "token.hpp"
#include "CommandModel.hpp"
#include <vector>
#include <optional>

class Parser
{
private :
    std::optional<Command> Parse_Command(const std::vector<Token>& tokens);
    std::optional<Pipeline> Parse_Pipeline(const std::vector<Token>& tokens);
    std::optional<AndChain> Parse_Chain(const std::vector<Token>& tokens);
    
    bool is_redirection_token(const Token& token);
public :
    std::optional<Job> Parse_Job(const std::vector<Token>& tokens);
};