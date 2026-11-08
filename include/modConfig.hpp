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