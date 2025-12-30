#include "main.hpp"

#include "scotland2/shared/modloader.h"

#include "metacore/shared/input.hpp"
#include "metacore/shared/events.hpp"
#include "metacore/shared/internals.hpp"
#include "GlobalNamespace/AudioTimeSyncController.hpp"
#include "GlobalNamespace/GameSongController.hpp"
#include "GlobalNamespace/PauseController.hpp"
#include "GlobalNamespace/IGamePause.hpp"
#include "GlobalNamespace/BeatmapObjectManager.hpp"
#include "GlobalNamespace/BeatmapCallbacksController.hpp"
#include "GlobalNamespace/NoteCutSoundEffectManager.hpp"
#include "GlobalNamespace/NoteCutSoundEffect.hpp"
#include "GlobalNamespace/IBeatmapObjectController.hpp"
#include "GlobalNamespace/MemoryPoolContainer_1.hpp"
#include "GlobalNamespace/CallbacksInTime.hpp"
#include "UnityEngine/Resources.hpp"
#include "UnityEngine/GameObject.hpp"
#include "System/Collections/Generic/Dictionary_2.hpp"
#include "System/Collections/Generic/LinkedList_1.hpp"
#include "System/Collections/Generic/LinkedListNode_1.hpp"
#include "UnityEngine/AudioSource.hpp"
#include "UnityEngine/AudioClip.hpp"
#include "GlobalNamespace/BeatmapDataItem.hpp"

static modloader::ModInfo modInfo{MOD_ID, VERSION, 0};
// Stores the ID and version of our mod, and is sent to
// the modloader upon startup

// Loads the config from disk using our modInfo, then returns it for use
// other config tools such as config-utils don't use this config, so it can be
// removed if those are in use
Configuration &getConfig() {
    static Configuration config(modInfo);
    return config;
}

// Stop collisions, switch saber models, etc.
bool fullPause = true;
// Whether to show the three second countdown before resuming
// bool resumeAnimation = false;

UnityW<GlobalNamespace::AudioTimeSyncController> audioTimeSyncController = nullptr;
// UnityW<GlobalNamespace::GameSongController> gameSongController = nullptr;
UnityW<GlobalNamespace::PauseController> pauseController = nullptr;
UnityW<GlobalNamespace::NoteCutSoundEffectManager> noteCutSoundEffectManager = nullptr;
GlobalNamespace::IGamePause* gamePause = nullptr;
GlobalNamespace::BeatmapObjectManager* beatmapObjectManager = nullptr;
GlobalNamespace::BeatmapCallbacksController* beatmapCallbacksController = nullptr;

bool findImportantStuff() {
    #define findObject(name, source)                                        \
    name = source;                                                          \
    if(!name) {PaperLogger.error("Could not find " #name); return false;} 

    findObject(audioTimeSyncController, UnityEngine::Object::FindObjectOfType<GlobalNamespace::AudioTimeSyncController*>());
    // findObject(gameSongController, UnityEngine::Object::FindObjectOfType<GlobalNamespace::GameSongController*>());
    findObject(pauseController, UnityEngine::Object::FindObjectOfType<GlobalNamespace::PauseController*>());
    findObject(noteCutSoundEffectManager, UnityEngine::Object::FindObjectOfType<GlobalNamespace::NoteCutSoundEffectManager*>());
    findObject(gamePause, pauseController->_gamePause);
    findObject(beatmapObjectManager, MetaCore::Internals::beatmapObjectManager); // Must be renewed each time the game loads
    findObject(beatmapCallbacksController, MetaCore::Internals::beatmapCallbacksController); // Must be renewed each time the game loads

    return true;
    #undef getObject
}

void stopAllNoteCutSoundEffects() {
    for(int i = 0; i < noteCutSoundEffectManager->_noteCutSoundEffectPoolContainer->get_activeItems()->get_Count(); i++) {
        auto noteCutSoundEffect = noteCutSoundEffectManager->_noteCutSoundEffectPoolContainer->get_activeItems()->get_Item(i);
        noteCutSoundEffect->StopPlayingAndFinish();
    }
    noteCutSoundEffectManager->_prevNoteATime = 0;
    noteCutSoundEffectManager->_prevNoteBTime = 0;
}

void resetBeatmap() {
    PaperLogger.debug("beatmapObjectManager->_allBeatmapObjects->get_Count: {}", beatmapObjectManager->_allBeatmapObjects->get_Count());
    for(int i = 0; i < beatmapObjectManager->_allBeatmapObjects->get_Count(); i++) {
        auto beatmapObject = beatmapObjectManager->_allBeatmapObjects->get_Item(i);
        // Dissolve instead of disable to trigger full despawn
        reinterpret_cast<UnityEngine::MonoBehaviour*>(beatmapObject)->get_gameObject()->SetActive(true);
        beatmapObject->Dissolve(0);
    }

    PaperLogger.debug("# beatmapCallbacksController->_callbacksInTimes->_entries: {}", beatmapCallbacksController->_callbacksInTimes->_entries.size());
    PaperLogger.debug("First BeatmapDataItem time: {}", beatmapCallbacksController->_beatmapData->get_allBeatmapDataItems()->get_First()->get_Value()->get_time());
    for(auto callbacksInTime : beatmapCallbacksController->_callbacksInTimes->_entries) {
        callbacksInTime.value->lastProcessedNode = nullptr; // Fault
    }
    beatmapCallbacksController->_prevSongTime = 0;
}

void manualPause() {
    if(fullPause) gamePause->Pause();
    else audioTimeSyncController->Pause();
    stopAllNoteCutSoundEffects();
}

void manualResume() {
    if(fullPause) gamePause->Resume();
    else audioTimeSyncController->Resume();
    // audioTimeSyncController will make it so pause + rewind prohibits unpausing
    // gameSongController will make it so BeatmapCallbacksUpdater also pauses, stopping the notes from spawning while seeking
}

void manualSeekToAbsolute(float songTime) {
    PaperLogger.debug("Seeking to absolute time: {}", songTime);
    PaperLogger.debug("audioTimeSyncController state: {}", static_cast<int>(audioTimeSyncController->get_state()));
    PaperLogger.debug("audioTimeSyncController audioLatency: {}", audioTimeSyncController->_audioLatency);
    PaperLogger.debug("audioSource is playing ({}) at: {}", audioTimeSyncController->_audioSource->get_isPlaying(), audioTimeSyncController->_audioSource->get_time());
    PaperLogger.debug("audioSource length: {}", audioTimeSyncController->_audioSource->clip->get_length());
    audioTimeSyncController->_startSongTime = std::max(0.0f, songTime);
    audioTimeSyncController->SeekTo(0);
    resetBeatmap();
    stopAllNoteCutSoundEffects();
}

void handleAOnPress() {
    if(!MetaCore::Input::GetPressed(MetaCore::Input::Controllers::Right, MetaCore::Input::Buttons::AX)) return;
    PaperLogger.debug("A Pressed");

    if(!findImportantStuff()) return;

    if(!MetaCore::Input::GetPressed(MetaCore::Input::Controllers::Left, MetaCore::Input::Buttons::AX)) {
        manualPause();
    } else {
        float offset = -1;
        manualSeekToAbsolute(audioTimeSyncController->get_songTime() + offset);
    }
}

void handleBOnPress() {
    if(!MetaCore::Input::GetPressed(MetaCore::Input::Controllers::Right, MetaCore::Input::Buttons::BY)) return;
    PaperLogger.debug("B Pressed");

    if(!findImportantStuff()) return;

    PaperLogger.debug("audioTimeSyncController state: {}", static_cast<int>(audioTimeSyncController->get_state()));
    PaperLogger.debug("audioTimeSyncController audioLatency: {}", audioTimeSyncController->_audioLatency);
    PaperLogger.debug("audioSource is playing ({}) at: {}", audioTimeSyncController->_audioSource->get_isPlaying(), audioTimeSyncController->_audioSource->get_time());
    PaperLogger.debug("audioSource length: {}", audioTimeSyncController->_audioSource->clip->get_length());
    manualResume();
}

// Called at the early stages of game loading
MOD_EXTERN_FUNC void setup(CModInfo *info) noexcept {
    *info = modInfo.to_c();

    getConfig().Load();

    // File logging
    Paper::Logger::RegisterFileContextId(PaperLogger.tag);

    PaperLogger.info("Completed setup!");
}

// Called later on in the game loading - a good time to install function hooks
MOD_EXTERN_FUNC void late_load() noexcept {
    il2cpp_functions::Init();

    MetaCore::Events::AddCallback(MetaCore::Input::PressEvents, MetaCore::Input::Buttons::AX, handleAOnPress);
    MetaCore::Events::AddCallback(MetaCore::Input::PressEvents, MetaCore::Input::Buttons::BY, handleBOnPress);

    PaperLogger.info("Installing hooks...");

    PaperLogger.info("Installed all hooks!");
}