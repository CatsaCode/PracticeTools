#pragma once

#include "config-utils/shared/config-utils.hpp"

DECLARE_CONFIG(PracticeToolsConfig) {
    CONFIG_VALUE(enabled, bool, "enabled", true);
    CONFIG_VALUE(fullPause, bool, "fullPause", false);
    CONFIG_VALUE(skipDuration, float, "skipDuration", 3);
    CONFIG_VALUE(speedChangePercentage, float, "speedChangePercentage", 10);
    CONFIG_VALUE(deadband, float, "deadband", 0.1);
    CONFIG_VALUE(sensitivity, float, "sensitivity", 1);

    CONFIG_VALUE(buttonXOnClick, int, "buttonXOnClick", 0);
    CONFIG_VALUE(buttonXOnHold, int, "buttonXOnHold", 0);
    CONFIG_VALUE(buttonYOnClick, int, "buttonYOnClick", 0);
    CONFIG_VALUE(buttonYOnHold, int, "buttonYOnHold", 0);
    CONFIG_VALUE(leftTriggerOnClick, int, "leftTriggerOnClick", 0);
    CONFIG_VALUE(leftTriggerOnHold, int, "leftTriggerOnHold", 0);
    CONFIG_VALUE(leftGripOnClick, int, "leftGripOnClick", 0);
    CONFIG_VALUE(leftGripOnHold, int, "leftGripOnHold", 0);
    CONFIG_VALUE(leftThumbstickOnClick, int, "leftThumbstickOnClick", 0);
    CONFIG_VALUE(leftThumbstickOnHold, int, "leftThumbstickOnHold", 0);
    CONFIG_VALUE(leftThumbstickHorizontalOnUpdate, int, "leftThumbstickHorizontalOnUpdate", 0);
    CONFIG_VALUE(leftThumbstickVerticalOnUpdate, int, "leftThumbstickVerticalOnUpdate", 0);

    CONFIG_VALUE(buttonAOnClick, int, "buttonAOnClick", 0);
    CONFIG_VALUE(buttonAOnHold, int, "buttonAOnHold", 0);
    CONFIG_VALUE(buttonBOnClick, int, "buttonBOnClick", 0);
    CONFIG_VALUE(buttonBOnHold, int, "buttonBOnHold", 0);
    CONFIG_VALUE(rightTriggerOnClick, int, "rightTriggerOnClick", 0);
    CONFIG_VALUE(rightTriggerOnHold, int, "rightTriggerOnHold", 0);
    CONFIG_VALUE(rightGripOnClick, int, "rightGripOnClick", 0);
    CONFIG_VALUE(rightGripOnHold, int, "rightGripOnHold", 0);
    CONFIG_VALUE(rightThumbstickOnClick, int, "rightThumbstickOnClick", 0);
    CONFIG_VALUE(rightThumbstickOnHold, int, "rightThumbstickOnHold", 0);
    CONFIG_VALUE(rightThumbstickHorizontalOnUpdate, int, "rightThumbstickHorizontalOnUpdate", 0);
    CONFIG_VALUE(rightThumbstickVerticalOnUpdate, int, "rightThumbstickVerticalOnUpdate", 0);
};