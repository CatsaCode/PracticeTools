#pragma once

namespace PracticeTools::ControllerFunctions {

    enum class ClickFunction {
        None,
        SkipForward, SkipBackward,
        IncreaseSpeed, DecreaseSpeed, ResetSpeed,
        PausePlay,
        AddMarker, RemoveMarker,
        NextMarker, PreviousMarker
    };

    enum class HoldFunction {
        None,
        SkipForward, SkipBackward,
        IncreaseSpeed, DecreaseSpeed,
    };

    enum class ThumbstickFunction {
        None,
        TimeSkip,
        AdjustSpeed
    };

    void execute(ClickFunction function);
    void execute(HoldFunction function, float input);
    void execute(ThumbstickFunction function, float input);

}