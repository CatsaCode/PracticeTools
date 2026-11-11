#pragma once

#include <string_view>

namespace PracticeTools::ControllerFunctions {

    enum class ClickFunction {
        None,
        SkipForward, SkipBackward,
        IncreaseSpeed, DecreaseSpeed, ResetSpeed,
        PausePlay,
        AddMarker, RemoveMarker,
        NextMarker, PreviousMarker
    };
    // TODO Mark these as constexpr once BSML is updated
    static /*constexpr*/ std::string_view clickFunctionLabels[] = {
        "None",
        "Skip Forward", "Skip Backward",
        "Increase Speed", "Decrease Speed", "Reset Speed",
        "Pause / Play",
        "Add Marker", "Remove Marker",
        "Next Marker", "Previous Marker"
    };

    enum class HoldFunction {
        None,
        SkipForward, SkipBackward,
        IncreaseSpeed, DecreaseSpeed,
    };
    static /*constexpr*/ std::string_view holdFunctionLabels[] = {
        "None",
        "Skip Forward", "Skip Backward",
        "Increase Speed", "Decrease Speed",
    };

    enum class ThumbstickFunction {
        None,
        TimeSkip,
        AdjustSpeed
    };
    static /*constexpr*/ std::string_view thumbstickFunctionLabels[] = {
        "None",
        "Time Skip",
        "Adjust Speed"
    };

    void execute(ClickFunction function);
    void execute(HoldFunction function, float input);
    void execute(ThumbstickFunction function, float input);

}