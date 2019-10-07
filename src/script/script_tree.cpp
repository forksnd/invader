// SPDX-License-Identifier: GPL-3.0-only

#include <optional>
#include "script_tree.hpp"

namespace Invader {
    // Get a principal object (script or global)
    static std::optional<ScriptTree::Object> get_principal_object(const Tokenizer::Token *first_token, const Tokenizer::Token *last_token, std::size_t &advance, bool &error, std::size_t &error_line, std::size_t &error_column, std::string &error_token, std::string &error_message);

    std::vector<ScriptTree::Object> ScriptTree::compile_script_tree(const std::vector<Tokenizer::Token> &tokens, bool &error, std::size_t &error_line, std::size_t &error_column, std::string &error_token, std::string &error_message) {
        std::vector<Object> objects;
        error = false;

        // Iterate through each token
        std::size_t token_count = tokens.size();
        auto *last_token = tokens.end().base();
        for(std::size_t i = 0; i < token_count; i++) {
            // See if we have something
            auto &token = tokens[i];
            if(token.type == Tokenizer::Token::TYPE_PARENTHESIS_OPEN) {
                std::size_t advance;
                auto object = get_principal_object(&token, last_token, advance, error, error_line, error_column, error_token, error_message);
                if(error) {
                    return std::vector<Object>();
                }
                else {
                    i += advance;
                    i--;
                }
                objects.push_back(object.value());
            }

            // Otherwise, bail
            else {
                error = true;
                error_message = "Unexpected token (expected an opening parenthesis)";
                error_line = token.line;
                error_column = token.column;
                error_token = token.raw_value;
                return std::vector<ScriptTree::Object>();
            }
        }

        return objects;
    }

    static std::optional<ScriptTree::Object> get_global(const Tokenizer::Token *first_token, std::size_t advance, bool &error, std::size_t &error_line, std::size_t &error_column, std::string &error_token, std::string &error_message);
    static std::optional<ScriptTree::Object> get_script(const Tokenizer::Token *first_token, std::size_t advance, bool &error, std::size_t &error_line, std::size_t &error_column, std::string &error_token, std::string &error_message);

    #define RETURN_ERROR_TOKEN(token, error_msg) \
        error = true; \
        error_line = (token).line; \
        error_column = (token).column; \
        error_token = (token).raw_value; \
        error_message = error_msg; \
        return std::nullopt;

    static std::optional<ScriptTree::Object> get_principal_object(const Tokenizer::Token *first_token, const Tokenizer::Token *last_token, std::size_t &advance, bool &error, std::size_t &error_line, std::size_t &error_column, std::string &error_token, std::string &error_message) {
        // Get the remaining amount of tokens
        std::size_t remaining = last_token - first_token;

        // Get how big our block is
        std::size_t depth = 0;
        for(advance = 0; advance < remaining; advance++) {
            auto &token = first_token[advance];
            if(token.type == Tokenizer::Token::TYPE_PARENTHESIS_OPEN) {
                depth++;
            }
            else if(token.type == Tokenizer::Token::TYPE_PARENTHESIS_CLOSE) {
                depth--;
                if(depth == 0) {
                    advance++;
                    break;
                }
            }
        }

        // If the depth is non-zero, it's unterminated
        if(depth > 0) {
            RETURN_ERROR_TOKEN(*first_token, "Unterminated script block");
        }

        // We need at least enough room for the script/global definition and the parenthesis
        if(advance < 5) {
            RETURN_ERROR_TOKEN(*first_token, "Incomplete script or global definition");
        }

        auto &type_token = first_token[1];
        if(type_token.type != Tokenizer::Token::TYPE_STRING) {
            RETURN_ERROR_TOKEN(type_token, "Non-string script or global type");
        }
        auto &type = std::get<std::string>(type_token.value);

        if(type == "script") {
            return get_script(first_token, advance, error, error_line, error_column, error_token, error_message);
        }
        else if(type == "global") {
            return get_global(first_token, advance, error, error_line, error_column, error_token, error_message);
        }
        else {
            RETURN_ERROR_TOKEN(type_token, "Expected \"global\" or \"script\"");
        }
    }

    static std::optional<ScriptTree::Object> get_script(const Tokenizer::Token *first_token, std::size_t advance, bool &error, std::size_t &error_line, std::size_t &error_column, std::string &error_token, std::string &error_message) {
        return std::nullopt;
    }

    static std::optional<ScriptTree::Object> get_global(const Tokenizer::Token *first_token, std::size_t advance, bool &error, std::size_t &error_line, std::size_t &error_column, std::string &error_token, std::string &error_message) {
        // Make sure we have a complete global definition: "(" "global" <type> <name> <value> ")"
        if(advance < 6) {
            RETURN_ERROR_TOKEN(*first_token, "Incomplete global definition");
        }

        // Begin!
        ScriptTree::Object::Global global;

        #define SET_GLOBAL_STRING_OR_BAIL(token, str) \
            if(token.type != Tokenizer::Token::TYPE_STRING) { \
                RETURN_ERROR_TOKEN(token, "Expected a string"); \
            } \
            global.str = std::get<std::string>(token.value);

        // Get the global type
        auto &global_type = first_token[2];
        if(global_type.type != Tokenizer::Token::TYPE_STRING) {
            RETURN_ERROR_TOKEN(first_token[2], "Expected a string");
        }
        global.global_type = HEK::string_to_value_type(std::get<std::string>(global_type.value).data());
        if(global.global_type <= HEK::ScenarioScriptValueType::SCENARIO_SCRIPT_VALUE_TYPE_VOID) {
            RETURN_ERROR_TOKEN(first_token[2], "Invalid global type");
        }

        auto &global_name = first_token[3];
        if(global_name.type != Tokenizer::Token::TYPE_STRING) {
            RETURN_ERROR_TOKEN(first_token[3], "Expected a string");
        }

        global.value = first_token[4];

        #undef SET_GLOBAL_STRING_OR_BAIL

        // Initialize and get everything going
        std::optional<ScriptTree::Object> r = ScriptTree::Object();
        auto &global_object = r.value();
        global_object.type = ScriptTree::Object::Type::TYPE_GLOBAL;
        global_object.value = global;

        return r;
    }

    #undef RETURN_ERROR_TOKEN
}