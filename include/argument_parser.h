#pragma once

#include <unordered_map>
#include <variant>

namespace asciimediaplayer {

    class ArgException : public std::exception {
      public:
        ArgException(const std::string& message) : message(std::string("ArgumentParser::") + message) {}

        const char* what() const noexcept {
            return message.c_str();
        }

      private:
        std::string message;
    };

    enum class ArgumentType {
        Void,
        Int,
        String
    };


    class ArgumentParser {

        using ArgumentValue = std::variant<std::monostate, int, unsigned int, std::string>;
        
        struct Flag {
            char name;
            std::string alias;
            ArgumentType type;
            bool required;
        };

        inline bool isValidAlias(const std::string alias) {
            if (!(alias.size() > 2 && alias[0] == '-' && alias[1] == '-' && std::isalpha(alias[2]))) {
                return false;
            }
            for (auto i = 3u; i < alias.size(); ++i) {
                if (!std::isalpha(alias[i]) && alias[i] != '-' && alias[i] != '_') {
                    return false;
                }
            }
            return true;
        }

        inline Flag getFlag(const std::string& flagStr) {
            if (flagStr.size() == 2 && flagStr[0] == '-' && std::isalpha(flagStr[1])) {
                if (!flags.contains(flagStr[1])) {
                    throw ArgException(std::string("parse: invalid flag: '") + flagStr + "'");
                }
                return flags[flagStr[1]];
            } else if (isValidAlias(flagStr)) {
                if (!aliasTable.contains(flagStr)) {
                    throw ArgException(std::string("parse: invalid flag: '") + flagStr + "'");
                }
                return flags[aliasTable[flagStr]];
            } else {
                throw ArgException(std::string("parse: flag expected, but got '") + flagStr + "'");
            }
        }

      public:
        
        /**
         * adds an argument to parser
         *
         * @param flag must start with '-' and contain only one letter
         * @param type type of the value of the argument, see ArgumentType for possible types
         * @param required true if argument is required, false if argument is optional
         */
        inline void addArgument(const std::string& flag, ArgumentType type, bool required = false) {
            if (!(flag.length() == 2 && flag[0] == '-' && std::isalpha(flag[1]))) {
                throw ArgException("addArgument: flag must start with '-' and contain only one letter");
            }
            if (flags.contains(flag[1])) {
                throw ArgException(std::string("addArgument: flag '-") + flag[1] + "' already exists");
            }
            flags[flag[1]] = { flag[1], std::string{}, type, required };
        }
        
         /**
         * adds an argument to parser
         *
         * @param flag must start with '-' and contain only one letter
         * @param alias must start with '--' and contain letter followed by letters, '-' and '_'
         * @param type type of the value of the argument, see ArgumentType for possible types
         * @param required true if argument is required, false if argument is optional
         */
        inline void addArgument(const std::string& flag, const std::string& alias, ArgumentType type, bool required = false) {
            if (!(flag.size() == 2 && flag[0] == '-' && std::isalpha(flag[1]))) {
                throw ArgException("addArgument: flag must start with '-' and contain only one letter");
            }
            auto aliasAccepted = [](const std::string& alias) {
                if (!(alias.size() > 2 && alias[0] == '-' && alias[1] == '-' && std::isalpha(alias[2]))) {
                    return false;
                }
                for (auto i = 3u; i < alias.size(); ++i) {
                    if (!std::isalpha(alias[i]) && alias[i] != '-' && alias[i] != '_') {
                        return false;
                    }
                }
                return true;
            };
            if (!aliasAccepted(alias)) {
                throw ArgException("addArgument: long alias must start with '--' and contain letter followed by letters, '-' and '_'");
            }
            if (flags.contains(flag[1])) {
                throw ArgException(std::string("addArgument: flag '-") + flag[1] + "' already exists");
            }
            if (aliasTable.contains(alias)) {
                throw ArgException(std::string("addArgument: alias '") + alias + "' is already used for flag '-" + aliasTable[alias] + '\'');
            }
            flags[flag[1]] = { flag[1], alias, type, required };
            aliasTable[alias] = flag[1];
        }

        /**
         * parses the given array of arguments
         *
         * @param argc from main
         * @param argv from main
         */
        void parse(int argc, const char* argv[]) {
            enum State {
                Default,
                WaitingForValue
            };
            State state = Default;
            Flag flag;

            for (auto i = 1; i < argc; ++i) {
                switch (state) {
                    case Default:
                        flag = getFlag(argv[i]);
                        if (argumentValues.contains(flag.name)) {
                            throw ArgException(std::string{ "parse: double used flag '-" } + flag.name + (flag.alias.empty() ? std::string{} : std::string{ "/" } + flag.alias) + "'");
                        }
                        if (flag.type == ArgumentType::Void) {
                            argumentValues[flag.name] = std::monostate{};
                        } else {
                            state = WaitingForValue;
                        }
                        break;
                    case WaitingForValue:
                        if (flag.type == ArgumentType::Int) {
                            try {
                                argumentValues[flag.name] = std::stoi(argv[i]);
                            } catch (const std::invalid_argument&) {
                                throw ArgException(std::string{ "parse: invalid conversion: '" } + argv[i] + "' to int");
                            } catch (const std::out_of_range&) {
                                throw ArgException(std::string{ "parse: failed conversion to int: '" } + argv[i] + "' is out of range");
                            }
                        } else {
                            argumentValues[flag.name] = std::string{ argv[i] };
                        }
                        state = Default;
                }
            }

            if (state == WaitingForValue) {
                throw ArgException(std::string{ "parse: flag '" } + flag.name + (flag.alias.empty() ? std::string{} : std::string{ "/" } + flag.alias) + "' needs a value");
            }

            for (const auto& [ charFlag, flag ] : flags) {
                if (flag.required && !argumentValues.contains(flag.name)) {
                    throw ArgException(std::string{ "parse: required flag '-" } + flag.name + (flag.alias.empty() ? std::string{} : std::string{ "/" } + flag.alias) + "' missing");
                }
            }
        }

        template<ArgumentType requiredType>
        const auto get(const std::string& flag) {
            char charFlag;
            if (flag.size() == 2 && flag[0] == '-' && std::isalpha(flag[1])) {
                charFlag = flag[1];
            } else if (aliasTable.contains(flag)) {
                charFlag = aliasTable[flag];
            } else {
                throw ArgException(std::string("get: wrong flag: '") + flag + "'");
            }
            if (!flags.contains(charFlag)) {
                throw ArgException(std::string("get: wrong flag: '") + flag + "'");
            }
            if (flags[charFlag].type != requiredType) {
                auto typeString = [](ArgumentType type) {
                    switch (type) {
                        case ArgumentType::Void:
                            return "Void";
                        case ArgumentType::Int:
                            return "Int";
                        default:
                            return "String";
                    }
                };
                throw ArgException(std::string("get: trying to get '") + typeString(requiredType) +
                        "' value for flag '" + flag + "', the type of which is '" + typeString(requiredType) + "'");
            }
            if constexpr (requiredType == ArgumentType::Void) {
                return argumentValues.contains(charFlag);
            } else if constexpr (requiredType == ArgumentType::Int) {
                if (argumentValues.contains(charFlag)) {
                    return std::make_pair(true, std::get<int>(argumentValues[charFlag]));
                } else {
                    return std::make_pair(false, 0);
                }
            } else {
                if (argumentValues.contains(charFlag)) {
                    return std::make_pair(true, std::get<std::string>(argumentValues[charFlag]));
                } else {
                    return std::make_pair(false, std::string{});
                }
            }
        }

      private:
        
        std::unordered_map<char, Flag> flags;
        std::unordered_map<std::string, char> aliasTable;
        std::unordered_map<char, ArgumentValue> argumentValues;
    };

} // namespace asciimediaplayer

