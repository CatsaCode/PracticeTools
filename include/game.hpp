#pragma once

#include "GlobalNamespace/AudioTimeSyncController.hpp"

namespace PracticeTools::Game {
    extern UnityW<GlobalNamespace::AudioTimeSyncController> audioTimeSyncController;

    bool areShortcutsEnabled();
    bool initReferences();
    void resetReferences();
    void installHooks();

    void stopAllNoteCutSoundEffects();
    void despawnAllObjects();
    void manualPause();
    void manualResume();
    void manualSeekToAbsolute(float songTime);
    void manualSetTimeScale(float timeScale);
}