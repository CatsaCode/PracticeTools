#pragma once

#include "main.hpp"
#include "config.hpp"

#include <unordered_map>
#include <string_view>
#include <functional>

namespace PracticeTools::ControllerFunctions {
    template <typename T>
    using FunctionList = std::unordered_map<std::string_view, std::function<T>>;

    extern FunctionList<void()> clickFunctions;
    extern FunctionList<void(float)> holdFunctions;
    extern FunctionList<void(float)> thumbstickFunctions;

    void invoke(FunctionList<void()>& list, std::string_view name);

    template <typename T>
    void invoke(FunctionList<void(T)>& list, std::string_view name, T input) {
        if(name.empty()) return;
        if(abs(input) < getPracticeToolsConfig().deadband.GetValue()) return;
        if(!list.contains(name)) {PaperLogger.error("Could not invoke function: '{}'", name); return;}
        list[name](input);
    }
}