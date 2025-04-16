#include "ArgParser.h"

namespace ArgumentParser {


void BaseArgument::SetShortName(char shortName) {
    short_name_ = shortName;
}

char BaseArgument::GetShortName() const {
    return short_name_;
}

void BaseArgument::SetDescription(const std::string& description) {
    description_ = description;
}

std::string BaseArgument::GetDescription() const {
    return description_;
}

void BaseArgument::SetRequired(bool required) {
    is_required_ = required;
}

bool BaseArgument::IsRequired() const {
    return is_required_;
}

void BaseArgument::SetPositional(bool positional) {
    is_positional_ = positional;
}

bool BaseArgument::IsPositional() const {
    return is_positional_;
}

void BaseArgument::SetMultiValue(bool multi, size_t minCount) {
    is_multi_value_ = multi;
    min_args_count_ = minCount;
}

bool BaseArgument::IsMultiValue() const {
    return is_multi_value_;
}

size_t BaseArgument::GetMinArgsCount() const {
    return min_args_count_;
}


bool StringArgument::Parse(const std::string& value) {
    values_.push_back(value);
    if (store_value_) *store_value_ = value;
    if (store_values_) store_values_->push_back(value);
    return true;
}

bool IntArgument::Parse(const std::string& value) {
    try {
        int parsed_value = std::stoi(value);
        values_.push_back(parsed_value);
        if (store_value_) *store_value_ = parsed_value;
        if (store_values_) store_values_->push_back(parsed_value);
        return true;
    } catch (...) {
        return false;
    }
}

FlagArgument::FlagArgument() {
    Default(false);
}

bool FlagArgument::Parse(const std::string& value) {
    bool parsed_value = true;
    if (!value.empty()) {
        parsed_value = (value == "true" || value == "1");
    }
    values_.push_back(parsed_value);
    if (store_value_) *store_value_ = parsed_value;
    if (store_values_) store_values_->push_back(parsed_value);
    return true;
}

ArgParser::ArgParser(const std::string& name) : name_(name) {}

StringArgument& ArgParser::AddStringArgument(const std::string& name) {
    return AddStringArgument(0, name, "");
}

StringArgument& ArgParser::AddStringArgument(char shortName, const std::string& name) {
    return AddStringArgument(shortName, name, "");
}

StringArgument& ArgParser::AddStringArgument(char shortName, const std::string& name, const std::string& description) {
    auto arg = std::make_shared<StringArgument>();
    arg->SetShortName(shortName);
    arg->SetDescription(description);
    arguments_[name] = arg;
    if (shortName) short_names_[shortName] = name;
    return *static_cast<StringArgument*>(arguments_[name].get());
}

IntArgument& ArgParser::AddIntArgument(const std::string& name) {
    return AddIntArgument(0, name, "");
}

IntArgument& ArgParser::AddIntArgument(char shortName, const std::string& name) {
    return AddIntArgument(shortName, name, "");
}

IntArgument& ArgParser::AddIntArgument(char shortName, const std::string& name, const std::string& description) {
    auto arg = std::make_shared<IntArgument>();
    arg->SetShortName(shortName);
    arg->SetDescription(description);
    arguments_[name] = arg;
    if (shortName) short_names_[shortName] = name;
    return *static_cast<IntArgument*>(arguments_[name].get());
}

FlagArgument& ArgParser::AddFlag(const std::string& name) {
    return AddFlag(0, name, "");
}

FlagArgument& ArgParser::AddFlag(const std::string& name, const std::string& description) {
    return AddFlag(0, name, description);
}

FlagArgument& ArgParser::AddFlag(char shortName, const std::string& name) {
    return AddFlag(shortName, name, "");
}

FlagArgument& ArgParser::AddFlag(char shortName, const std::string& name, const std::string& description) {
    auto arg = std::make_shared<FlagArgument>();
    arg->SetShortName(shortName);
    arg->SetDescription(description);
    arguments_[name] = arg;
    if (shortName) short_names_[shortName] = name;
    return *static_cast<FlagArgument*>(arguments_[name].get());
}

bool ArgParser::AddHelp(char shortName, const std::string& name, const std::string& description) {
    help_short_name_ = shortName;
    help_name_ = name;
    help_description_ = description;
    return true;
}

bool ArgParser::Parse(const std::vector<std::string>& args) {
    if (args.empty()) return false;
    
    std::vector<std::string> positional_args;
    
    for (size_t i = 1; i < args.size(); ++i) {
        const std::string& arg = args[i];
        
        if (arg == "--" + help_name_ || (help_short_name_ && arg == "-" + std::string(1, help_short_name_))) {
            help_requested_ = true;
            return true;
        }
        
        if (arg[0] == '-') {
            if (arg[1] == '-') {

                std::string name = arg.substr(2);
                std::string value;
                
                auto eq_pos = name.find('=');
                if (eq_pos != std::string::npos) {
                    value = name.substr(eq_pos + 1);
                    name = name.substr(0, eq_pos);
                } else if (i + 1 < args.size() && args[i + 1][0] != '-') {
                    value = args[++i];
                }
                
                if (!ParseArgument(name, value)) {
                    return false;
                }
            } else {
                std::string shortFlags = arg.substr(1);
                
                if (shortFlags.length() == 1 && i + 1 < args.size() && args[i + 1][0] != '-') {
                    char shortName = shortFlags[0];
                    if (short_names_.find(shortName) == short_names_.end()) {
                        return false;
                    }
                    if (!ParseArgument(short_names_[shortName], args[++i])) {
                        return false;
                    }
                } else {
                    for (char shortName : shortFlags) {
                        if (short_names_.find(shortName) == short_names_.end()) {
                            return false;
                        }
                        if (!ParseArgument(short_names_[shortName], "")) {
                            return false;
                        }
                    }
                }
            }
        } else {
            positional_args.push_back(arg);
        }
    }
    
    for (const auto& [name, arg] : arguments_) {
        if (arg->IsPositional()) {
            for (const auto& value : positional_args) {
                if (!ParseArgument(name, value)) {
                    return false;
                }
            }
            break;
        }
    }
    
    for (const auto& [name, arg] : arguments_) {
        if (arg->IsRequired() && !arg->IsSet()) {
            return false;
        }
        
        if (arg->IsMultiValue() && arg->GetMinArgsCount() > 0) {
            auto* string_arg = dynamic_cast<StringArgument*>(arg.get());
            if (string_arg && string_arg->GetValues().size() < arg->GetMinArgsCount()) {
                return false;
            }
            
            auto* int_arg = dynamic_cast<IntArgument*>(arg.get());
            if (int_arg && int_arg->GetValues().size() < arg->GetMinArgsCount()) {
                return false;
            }
        }
    }
    
    return true;
}

bool ArgParser::Parse(int argc, char** argv) {
    std::vector<std::string> args;
    for (int i = 0; i < argc; ++i) {
        args.push_back(argv[i]);
    }
    return Parse(args);
}

std::string ArgParser::GetStringValue(const std::string& name, size_t index) const {
    auto it = arguments_.find(name);
    if (it != arguments_.end()) {
        auto* arg = dynamic_cast<StringArgument*>(it->second.get());
        if (arg) return arg->GetValue(index);
    }
    return "";
}

int ArgParser::GetIntValue(const std::string& name, size_t index) const {
    auto it = arguments_.find(name);
    if (it != arguments_.end()) {
        auto* arg = dynamic_cast<IntArgument*>(it->second.get());
        if (arg) return arg->GetValue(index);
    }
    return 0;
}

bool ArgParser::GetFlag(const std::string& name) const {
    auto it = arguments_.find(name);
    if (it != arguments_.end()) {
        auto* arg = dynamic_cast<FlagArgument*>(it->second.get());
        if (arg) return arg->GetValue();
    }
    return false;
}

bool ArgParser::Help() const {
    return help_requested_;
}

std::string ArgParser::HelpDescription() const {
    std::string result = name_ + "\n\n";
    result += "Usage:\n";
    
    if (!help_name_.empty()) {
        result += "  --" + help_name_;
        if (help_short_name_) {
            result += ", -" + std::string(1, help_short_name_);
        }
        result += "  " + help_description_ + "\n";
    }
    
    for (const auto& [name, arg] : arguments_) {
        result += "  --" + name;
        if (arg->GetShortName()) {
            result += ", -" + std::string(1, arg->GetShortName());
        }
        if (!arg->GetDescription().empty()) {
            result += "  " + arg->GetDescription();
        }
        result += "\n";
    }
    
    return result;
}

bool ArgParser::ParseArgument(const std::string& name, const std::string& value) {
    auto it = arguments_.find(name);
    if (it == arguments_.end()) {
        return false;
    }
    
    return it->second->Parse(value);
}

}