#pragma once

#include "config-utils/shared/config-utils.hpp"

DECLARE_CONFIG(PracticeToolsConfig) {
    CONFIG_VALUE(enabled, bool, "enabled", true);
    CONFIG_VALUE(fullPause, bool, "fullPause", false);
    CONFIG_VALUE(skipDuration, float, "skipDuration", 3);
    CONFIG_VALUE(speedChangePercentage, float, "speedChangePercentage", 10);
    CONFIG_VALUE(deadband, float, "deadband", 0.1);
    CONFIG_VALUE(sensitivity, float, "sensitivity", 1);
};

// TODO Actual config and enums
namespace PracticeTools::ControllerCallbacks {
    static std::string buttonXOnClick = "Decrease Speed";
    static std::string buttonYOnClick = "Increase Speed";
    static std::string leftThumbstickOnClick = "Reset Speed";
    static std::string leftTriggerOnClick = "";
    static std::string leftGripOnClick = "";
    static std::string buttonXOnHold = "";
    static std::string buttonYOnHold = "";
    static std::string leftThumbstickOnHold = "";
    static std::string leftTriggerOnHold = "Decrease Speed";
    static std::string leftGripOnHold = "Skip Backward";
    static std::string leftThumbstickHorizontalOnUpdate = "";
    static std::string leftThumbstickVerticalOnUpdate = "";

    static std::string buttonAOnClick = "Skip Backward";
    static std::string buttonBOnClick = "Skip Forward";
    static std::string rightThumbstickOnClick = "Pause / Play";
    static std::string rightTriggerOnClick = "";
    static std::string rightGripOnClick = "";
    static std::string buttonAOnHold = "";
    static std::string buttonBOnHold = "";
    static std::string rightThumbstickOnHold = "";
    static std::string rightTriggerOnHold = "Increase Speed";
    static std::string rightGripOnHold = "Skip Forward";
    static std::string rightThumbstickHorizontalOnUpdate = "";
    static std::string rightThumbstickVerticalOnUpdate = "";
}