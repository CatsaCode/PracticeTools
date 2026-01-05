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
#include "GlobalNamespace/AudioManagerSO.hpp"
#include "GlobalNamespace/BeatmapDataItem.hpp"
#include "GlobalNamespace/NoteData.hpp"
#include "GlobalNamespace/NoteController.hpp"

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

// Config settings
// Whether or not shortcuts are enabled
bool isModEnabled = true;
// Stop collisions, hide saber models, etc.
bool fullPause = false;
// Whether to show the three second countdown before resuming
// bool resumeAnimation = false;

// References
static UnityW<GlobalNamespace::AudioTimeSyncController> audioTimeSyncController = nullptr;
static UnityW<GlobalNamespace::PauseController> pauseController = nullptr;
static UnityW<GlobalNamespace::NoteCutSoundEffectManager> noteCutSoundEffectManager = nullptr;
static UnityW<GlobalNamespace::AudioManagerSO> audioManager = nullptr;
static GlobalNamespace::IGamePause* gamePause = nullptr;
static GlobalNamespace::BeatmapObjectManager* beatmapObjectManager = nullptr;
static GlobalNamespace::BeatmapCallbacksController* beatmapCallbacksController = nullptr;

// State
bool isPracticing = false;
bool areReferencesSafe = false;
bool isTimeSkipping = false;


bool areShortcutsEnabled() {
    return isModEnabled && isPracticing && areReferencesSafe;
}

// Must be renewed each time the game loads
bool initReferences() {
    #define findObject(name, source)                                        \
    name = source;                                                          \
    if(!name) {PaperLogger.error("Could not find " #name); return false;} 

    areReferencesSafe = false;

    findObject(audioTimeSyncController, UnityEngine::Object::FindObjectOfType<GlobalNamespace::AudioTimeSyncController*>());
    findObject(pauseController, UnityEngine::Object::FindObjectOfType<GlobalNamespace::PauseController*>());
    findObject(noteCutSoundEffectManager, UnityEngine::Object::FindObjectOfType<GlobalNamespace::NoteCutSoundEffectManager*>());
    findObject(gamePause, pauseController->_gamePause);
    findObject(audioManager, noteCutSoundEffectManager->_audioManager);
    findObject(beatmapObjectManager, MetaCore::Internals::beatmapObjectManager);
    findObject(beatmapCallbacksController, MetaCore::Internals::beatmapCallbacksController);

    areReferencesSafe = true;
    return true;

    #undef getObject
}


MAKE_HOOK_MATCH(NoteCutSoundEffectManager_HandleNoteWasSpawned, &GlobalNamespace::NoteCutSoundEffectManager::HandleNoteWasSpawned,
    void, GlobalNamespace::NoteCutSoundEffectManager* self, GlobalNamespace::NoteController* noteController
) {
    if(!isTimeSkipping) return NoteCutSoundEffectManager_HandleNoteWasSpawned(self, noteController);
    
    GlobalNamespace::NoteData* noteData = noteController->get_noteData();
    if(!self->IsSupportedNote(noteData)) return;
    self->_prevNoteATime = noteData->get_time();
    self->_prevNoteBTime = noteData->get_time();
}

MAKE_HOOK_MATCH(BeatmapCallbacksController_ManualUpdate, &GlobalNamespace::BeatmapCallbacksController::ManualUpdate, void,
    GlobalNamespace::BeatmapCallbacksController* self, float songTime
) {
    BeatmapCallbacksController_ManualUpdate(self, songTime);
    isTimeSkipping = false;
}


void stopAllNoteCutSoundEffects() {
    for(int i = 0; i < noteCutSoundEffectManager->_noteCutSoundEffectPoolContainer->get_activeItems()->get_Count(); i++) {
        auto noteCutSoundEffect = noteCutSoundEffectManager->_noteCutSoundEffectPoolContainer->get_activeItems()->get_Item(i);
        noteCutSoundEffect->StopPlayingAndFinish();
    }
}

void despawnAllObjects() {
    for(int i = 0; i < beatmapObjectManager->_allBeatmapObjects->get_Count(); i++) {
        auto beatmapObject = beatmapObjectManager->_allBeatmapObjects->get_Item(i);
        // Dissolve instead of disable to trigger full despawn
        reinterpret_cast<UnityEngine::MonoBehaviour*>(beatmapObject)->get_gameObject()->SetActive(true);
        beatmapObject->Dissolve(0);
    }
}


void manualPause() {
    if(fullPause) gamePause->Pause();
    else audioTimeSyncController->Pause();
    stopAllNoteCutSoundEffects();
}

void manualResume() {
    if(fullPause) gamePause->Resume();
    else audioTimeSyncController->Resume();
}

void manualSeekToAbsolute(float songTime) {
    isTimeSkipping = true;
    bool reverse = songTime < audioTimeSyncController->get_songTime();

    songTime = std::max(0.0f, std::min(songTime + audioTimeSyncController->_audioLatency, audioTimeSyncController->_audioSource->get_clip()->get_length() - 0.01f));
    audioTimeSyncController->_startSongTime = songTime;
    audioTimeSyncController->SeekTo(0);

    beatmapCallbacksController->_prevSongTime = songTime;
    noteCutSoundEffectManager->_prevNoteATime = songTime;
    noteCutSoundEffectManager->_prevNoteBTime = songTime;

    despawnAllObjects();
    stopAllNoteCutSoundEffects();

    if(!reverse) return;
    for(auto callbacksInTime : beatmapCallbacksController->_callbacksInTimes->_entries) {
        while(callbacksInTime.value && callbacksInTime.value->lastProcessedNode != nullptr && callbacksInTime.value->lastProcessedNode->get_Value()->get_time() > songTime) {
            callbacksInTime.value->lastProcessedNode = callbacksInTime.value->lastProcessedNode->get_Previous();
        }
    }
}

// Will likely need to hook and rewrite AudioTimeSyncController::Update to remove any time fixing stuff if I want to try backwards audio
// I will also need a component or a more active hook to deal with the BeatmapCallbacksController callbacksInTimes
void manualSetTimeScale(float timeScale) {
    audioTimeSyncController->_timeScale = timeScale;
    audioTimeSyncController->_audioSource->set_pitch(timeScale);
    audioManager->musicPitch = 1 / timeScale;
    // TODO Adjust speed of NoteCutSoundEffects, or just stop them
}


void handleButtonAOnPress() {
    if(MetaCore::Input::GetPressed(MetaCore::Input::Controllers::Left, MetaCore::Input::Buttons::AX)) return;
    PaperLogger.debug("Button A");
    if(!areShortcutsEnabled()) return;

    if(!MetaCore::Input::GetPressed(MetaCore::Input::Controllers::Left, MetaCore::Input::Buttons::AX)) {
        // manualPause();
        manualSetTimeScale(audioTimeSyncController->_timeScale * 0.9);
    } else {
        float offset = -3;
        manualSeekToAbsolute(audioTimeSyncController->get_songTime() + offset);
    }
}

void handleButtonBOnPress() {
    if(MetaCore::Input::GetPressed(MetaCore::Input::Controllers::Left, MetaCore::Input::Buttons::BY)) return;
    PaperLogger.debug("Button B");
    if(!areShortcutsEnabled()) return;

    // manualResume();
    manualSetTimeScale(audioTimeSyncController->_timeScale / 0.9);
}

void handleButtonRightThumbstickOnPress() {
    if(MetaCore::Input::GetPressed(MetaCore::Input::Controllers::Left, MetaCore::Input::Buttons::Thumbstick)) return;
    PaperLogger.debug("Button Right Thumbstick");
    if(!areShortcutsEnabled()) return;
}

void handleButtonXOnPress() {
    if(MetaCore::Input::GetPressed(MetaCore::Input::Controllers::Right, MetaCore::Input::Buttons::AX)) return;
    PaperLogger.debug("Button X");
    if(!areShortcutsEnabled()) return;
}

void handleButtonYOnPress() {
    if(MetaCore::Input::GetPressed(MetaCore::Input::Controllers::Right, MetaCore::Input::Buttons::BY)) return;
    PaperLogger.debug("Button Y");
    if(!areShortcutsEnabled()) return;
}

void handleButtonLeftThumbstickOnPress() {
    if(MetaCore::Input::GetPressed(MetaCore::Input::Controllers::Right, MetaCore::Input::Buttons::Thumbstick)) return;
    PaperLogger.debug("Button Left Thumbstick");
    if(!areShortcutsEnabled()) return;
}

// Handle some controller buttons manually until MetaCore is fixed
bool isButtonXHeld = false;
bool isButtonYHeld = false;
bool isButtonLeftThumbstickHeld = false;
bool isButtonRightThumbstickHeld = false;
void checkControllerButtons() {
    if(!isButtonXHeld && MetaCore::Input::GetPressed(MetaCore::Input::Controllers::Left, MetaCore::Input::Buttons::AX)) handleButtonXOnPress();
    if(!isButtonYHeld && MetaCore::Input::GetPressed(MetaCore::Input::Controllers::Left, MetaCore::Input::Buttons::BY)) handleButtonYOnPress();
    if(!isButtonLeftThumbstickHeld && MetaCore::Input::GetPressed(MetaCore::Input::Controllers::Left, MetaCore::Input::Buttons::Thumbstick)) handleButtonLeftThumbstickOnPress();
    if(!isButtonRightThumbstickHeld && MetaCore::Input::GetPressed(MetaCore::Input::Controllers::Right, MetaCore::Input::Buttons::Thumbstick)) handleButtonRightThumbstickOnPress();

    isButtonXHeld = MetaCore::Input::GetPressed(MetaCore::Input::Controllers::Left, MetaCore::Input::Buttons::AX);
    isButtonYHeld = MetaCore::Input::GetPressed(MetaCore::Input::Controllers::Left, MetaCore::Input::Buttons::BY);
    isButtonLeftThumbstickHeld = MetaCore::Input::GetPressed(MetaCore::Input::Controllers::Left, MetaCore::Input::Buttons::Thumbstick);
    isButtonRightThumbstickHeld = MetaCore::Input::GetPressed(MetaCore::Input::Controllers::Right, MetaCore::Input::Buttons::Thumbstick);
}

void handleMapStart() {
    initReferences();
    isPracticing = true; // TODO Detect if in practice or normal mode
}

void handleMapUpdate() {
    checkControllerButtons();
}

void handleMapEnd() {
    isPracticing = false;
    areReferencesSafe = false;
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

    MetaCore::Events::AddCallback(MetaCore::Events::MapStarted, handleMapStart);
    MetaCore::Events::AddCallback(MetaCore::Events::Update, handleMapUpdate);
    MetaCore::Events::AddCallback(MetaCore::Events::MapEnded, handleMapEnd);
    MetaCore::Events::AddCallback(MetaCore::Input::PressEvents, MetaCore::Input::Buttons::AX, handleButtonAOnPress);
    MetaCore::Events::AddCallback(MetaCore::Input::PressEvents, MetaCore::Input::Buttons::BY, handleButtonBOnPress);
    // Handle these controller inputs manually until MetaCore is fixed
    // MetaCore::Events::AddCallback(MetaCore::Input::PressEvents, MetaCore::Input::Buttons::Thumbstick, handleButtonRightThumbstickOnPress);
    // MetaCore::Events::AddCallback(MetaCore::Input::PressEvents, MetaCore::Input::Buttons::AX, handleButtonXOnPress);
    // MetaCore::Events::AddCallback(MetaCore::Input::PressEvents, MetaCore::Input::Buttons::BY, handleButtonYOnPress);
    // MetaCore::Events::AddCallback(MetaCore::Input::PressEvents, MetaCore::Input::Buttons::Thumbstick, handleButtonLeftThumbstickOnPress);

    PaperLogger.info("Installing hooks...");

    INSTALL_HOOK(PaperLogger, NoteCutSoundEffectManager_HandleNoteWasSpawned);
    INSTALL_HOOK(PaperLogger, BeatmapCallbacksController_ManualUpdate);

    PaperLogger.info("Installed all hooks!");
}