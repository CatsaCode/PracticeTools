#pragma once

#include "controllerFunctions.hpp"

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
    using CF = ControllerFunctions::ClickFunction;
    using HF = ControllerFunctions::HoldFunction;
    using TF = ControllerFunctions::ThumbstickFunction;

    static CF buttonXOnClick =                    CF::DecreaseSpeed;
    static CF buttonYOnClick =                    CF::IncreaseSpeed;
    static CF leftThumbstickOnClick =             CF::ResetSpeed;
    static CF leftTriggerOnClick =                CF::None;
    static CF leftGripOnClick =                   CF::None;
    static HF buttonXOnHold =                     HF::None;
    static HF buttonYOnHold =                     HF::None;
    static HF leftThumbstickOnHold =              HF::None;
    static HF leftTriggerOnHold =                 HF::DecreaseSpeed;
    static HF leftGripOnHold =                    HF::SkipBackward;
    static TF leftThumbstickHorizontalOnUpdate =  TF::None;
    static TF leftThumbstickVerticalOnUpdate =    TF::None;

    static CF buttonAOnClick =                    CF::SkipBackward;
    static CF buttonBOnClick =                    CF::SkipForward;
    static CF rightThumbstickOnClick =            CF::PausePlay;
    static CF rightTriggerOnClick =               CF::None;
    static CF rightGripOnClick =                  CF::None;
    static HF buttonAOnHold =                     HF::None;
    static HF buttonBOnHold =                     HF::None;
    static HF rightThumbstickOnHold =             HF::None;
    static HF rightTriggerOnHold =                HF::IncreaseSpeed;
    static HF rightGripOnHold =                   HF::SkipForward;
    static TF rightThumbstickHorizontalOnUpdate = TF::None;
    static TF rightThumbstickVerticalOnUpdate =   TF::None;
}