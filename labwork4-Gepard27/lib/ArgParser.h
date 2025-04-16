#pragma once
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <functional>

namespace ArgumentParser {

class BaseArgument {
public:
    virtual ~BaseArgument() = default;
    virtual bool Parse(const std::string& value) = 0;
    virtual bool IsSet() const = 0;
    
    void SetShortName(char shortName);
    char GetShortName() const;
    
    void SetDescription(const std::string& description);
    std::string GetDescription() const;
    
    void SetRequired(bool required);
    bool IsRequired() const;
    
    void SetPositional(bool positional);
    bool IsPositional() const;
    
    void SetMultiValue(bool multi, size_t minCount = 0);
    bool IsMultiValue() const;
    size_t GetMinArgsCount() const;

protected:
    char short_name_ = 0;
    std::string description_;
    bool is_required_ = true;
    bool is_positional_ = false;
    bool is_multi_value_ = false;
    size_t min_args_count_ = 0;
};

template<typename T>
class Argument : public BaseArgument {
public:
    Argument& Default(const T& value);
    Argument& StoreValue(T& value);
    Argument& StoreValues(std::vector<T>& values);
    Argument& MultiValue(size_t minCount = 0);
    Argument& Positional();
    T GetValue(size_t index = 0) const;
    bool IsSet() const override;
    const std::vector<T>& GetValues() const;

protected:
    T default_value_{};
    bool has_default_ = false;
    std::vector<T> values_;
    T* store_value_ = nullptr;
    std::vector<T>* store_values_ = nullptr;
};

class StringArgument : public Argument<std::string> {
public:
    bool Parse(const std::string& value) override;
};

class IntArgument : public Argument<int> {
public:
    bool Parse(const std::string& value) override;
};

class FlagArgument : public Argument<bool> {
public:
    FlagArgument();
    bool Parse(const std::string& value) override;
};

class ArgParser {
public:
    ArgParser(const std::string& name);

    StringArgument& AddStringArgument(const std::string& name);
    StringArgument& AddStringArgument(char shortName, const std::string& name);
    StringArgument& AddStringArgument(char shortName, const std::string& name, const std::string& description);

    IntArgument& AddIntArgument(const std::string& name);
    IntArgument& AddIntArgument(char shortName, const std::string& name);
    IntArgument& AddIntArgument(char shortName, const std::string& name, const std::string& description);

    FlagArgument& AddFlag(const std::string& name);
    FlagArgument& AddFlag(const std::string& name, const std::string& description);
    FlagArgument& AddFlag(char shortName, const std::string& name);
    FlagArgument& AddFlag(char shortName, const std::string& name, const std::string& description);

    bool AddHelp(char shortName, const std::string& name, const std::string& description);
    bool Parse(const std::vector<std::string>& args);
    bool Parse(int argc, char** argv);
    std::string GetStringValue(const std::string& name, size_t index = 0) const;
    int GetIntValue(const std::string& name, size_t index = 0) const;
    bool GetFlag(const std::string& name) const;
    bool Help() const;
    std::string HelpDescription() const;

private:
    bool ParseArgument(const std::string& name, const std::string& value);

private:
    std::string name_;
    std::map<std::string, std::shared_ptr<BaseArgument>> arguments_;
    std::map<char, std::string> short_names_;
    char help_short_name_ = 0;
    std::string help_name_;
    std::string help_description_;
    bool help_requested_ = false;
};

template<typename T>
Argument<T>& Argument<T>::Default(const T& value) {
    default_value_ = value;
    has_default_ = true;
    is_required_ = false;
    return *this;
}

template<typename T>
Argument<T>& Argument<T>::StoreValue(T& value) {
    store_value_ = &value;
    return *this;
}

template<typename T>
Argument<T>& Argument<T>::StoreValues(std::vector<T>& values) {
    store_values_ = &values;
    return *this;
}

template<typename T>
Argument<T>& Argument<T>::MultiValue(size_t minCount) {
    SetMultiValue(true, minCount);
    return *this;
}

template<typename T>
Argument<T>& Argument<T>::Positional() {
    SetPositional(true);
    return *this;
}

template<typename T>
T Argument<T>::GetValue(size_t index) const {
    if (values_.empty()) {
        return default_value_;
    }
    return values_[index];
}

template<typename T>
bool Argument<T>::IsSet() const {
    return !values_.empty() || has_default_;
}

template<typename T>
const std::vector<T>& Argument<T>::GetValues() const {
    return values_;
}

}