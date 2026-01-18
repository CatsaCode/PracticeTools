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

#include <cstdlib>
#include <unordered_map>

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
float deadband = 0.1f;

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

// TODO Move to mod config
namespace ControllerCallbacks {
    std::string buttonXOnClick = "";
    std::string buttonYOnClick = "";
    std::string leftThumbstickOnClick = "";
    std::string leftTriggerOnClick = "";
    std::string leftGripOnClick = "";
    std::string buttonXOnHold = "";
    std::string buttonYOnHold = "";
    std::string leftThumbstickOnHold = "";
    std::string leftTriggerOnHold = "";
    std::string leftGripOnHold = "";
    std::string leftThumbstickHorizontalOnUpdate = "";
    std::string leftThumbstickVerticalOnUpdate = "";

    std::string buttonAOnClick = "";
    std::string buttonBOnClick = "";
    std::string rightThumbstickOnClick = "";
    std::string rightTriggerOnClick = "";
    std::string rightGripOnClick = "";
    std::string buttonAOnHold = "";
    std::string buttonBOnHold = "";
    std::string rightThumbstickOnHold = "";
    std::string rightTriggerOnHold = "";
    std::string rightGripOnHold = "";
    std::string rightThumbstickHorizontalOnUpdate = "";
    std::string rightThumbstickVerticalOnUpdate = "";
}


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

    songTime = std::max(0.0f, std::min(songTime, audioTimeSyncController->_audioSource->get_clip()->get_length() - 0.01f));
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


// TODO Implement all the controller callback functions
namespace ControllerFunctions {
    template <typename T>
    using FunctionList = std::unordered_map<std::string_view, std::function<T>>;

    FunctionList<void()> clickFunctions = {
        {"Skip Forward", [](){
            PaperLogger.debug("(Click) Skip Forward");
        }},
        {"Skip Backward", [](){
            PaperLogger.debug("(Click) Skip Backward");
        }},
        {"Increase Speed", [](){
            PaperLogger.debug("(Click) Increase Speed");
        }},
        {"Decrease Speed", [](){
            PaperLogger.debug("(Click) Decrease Speed");
        }},
        {"Reset Speed", [](){
            PaperLogger.debug("(Click) Reset Speed");
        }},
        {"Pause / Play", [](){
            PaperLogger.debug("(Click) Pause / Play");
        }},
        {"Add Marker", [](){
            PaperLogger.debug("(Click) Add Marker");
        }},
        {"Remove Marker", [](){
            PaperLogger.debug("(Click) Remove Marker");
        }},
        {"Next Marker", [](){
            PaperLogger.debug("(Click) Next Marker");
        }},
        {"Previous Marker", [](){
            PaperLogger.debug("(Click) Previous Marker");
        }}
    };

    FunctionList<void(float)> holdFunctions = {
        {"Skip Forward", [](float input){
            if(input < deadband) return;
            PaperLogger.debug("(Hold) Skip Forward: {}", input);
        }},
        {"Skip Backward", [](float input){
            if(input < deadband) return;
            PaperLogger.debug("(Hold) Skip Backward: {}", input);
        }},
        {"Increase Speed", [](float input){
            if(input < deadband) return;
            PaperLogger.debug("(Hold) Increase Speed: {}", input);
        }},
        {"Decrease Speed", [](float input){
            if(input < deadband) return;
            PaperLogger.debug("(Hold) Decrease Speed: {}", input);
        }}
    };

    FunctionList<void(float)> thumbstickFunctions = {
        {"Skip", [](float input){
            if(abs(input) < deadband) return;
            PaperLogger.debug("(Thumbstick) Skip: {}", input);
        }},
        {"Speed", [](float input){
            if(abs(input) < deadband) return;
            PaperLogger.debug("(Thumbstick) Speed: {}", input);
        }}
    };

    void invoke(FunctionList<void()>& list, std::string_view name) {
        if(name.empty()) return;
        if(!list.contains(name)) {PaperLogger.error("Could not invoke function: '{}'", name); return;}
        list[name]();
    }

    template <typename T>
    void invoke(FunctionList<void(T)>& list, std::string_view name, T input){
        if(name.empty()) return;
        if(!list.contains(name)) {PaperLogger.error("Could not invoke function: '{}'", name); return;}
        list[name](input);
    }
}


struct ControllerState {
    bool isButtonXHeld = false;
    bool isButtonYHeld = false;
    bool isLeftThumbstickHeld = false;
    bool isLeftTriggerHeld = false;
    bool isLeftGripHeld = false;
    bool isButtonAHeld = false;
    bool isButtonBHeld = false;
    bool isRightThumbstickHeld = false;
    bool isRightTriggerHeld = false;
    bool isRightGripHeld = false;

    float leftThumbstickHorizontalInput = 0;
    float leftThumbstickVerticalInput = 0;
    float leftTriggerInput = 0;
    float leftGripInput = 0;
    float rightThumbstickHorizontalInput = 0;
    float rightThumbstickVerticalInput = 0;
    float rightTriggerInput = 0;
    float rightGripInput = 0;

    void update() {
        isButtonXHeld         = MetaCore::Input::GetPressed(MetaCore::Input::Controllers::Left,  MetaCore::Input::Buttons::AX);
        isButtonYHeld         = MetaCore::Input::GetPressed(MetaCore::Input::Controllers::Left,  MetaCore::Input::Buttons::BY);
        isLeftThumbstickHeld  = MetaCore::Input::GetPressed(MetaCore::Input::Controllers::Left,  MetaCore::Input::Buttons::Thumbstick);
        isLeftTriggerHeld     = MetaCore::Input::GetPressed(MetaCore::Input::Controllers::Left,  MetaCore::Input::Buttons::Trigger);
        isLeftGripHeld        = MetaCore::Input::GetPressed(MetaCore::Input::Controllers::Left,  MetaCore::Input::Buttons::Grip);
        isButtonAHeld         = MetaCore::Input::GetPressed(MetaCore::Input::Controllers::Right, MetaCore::Input::Buttons::AX);
        isButtonBHeld         = MetaCore::Input::GetPressed(MetaCore::Input::Controllers::Right, MetaCore::Input::Buttons::BY);
        isRightThumbstickHeld = MetaCore::Input::GetPressed(MetaCore::Input::Controllers::Right, MetaCore::Input::Buttons::Thumbstick);
        isRightTriggerHeld    = MetaCore::Input::GetPressed(MetaCore::Input::Controllers::Right, MetaCore::Input::Buttons::Trigger);
        isRightGripHeld       = MetaCore::Input::GetPressed(MetaCore::Input::Controllers::Right, MetaCore::Input::Buttons::Grip);

        leftThumbstickHorizontalInput  = MetaCore::Input::GetAxis(MetaCore::Input::Controllers::Left,  MetaCore::Input::Axes::ThumbstickHorizontal);
        leftThumbstickVerticalInput    = MetaCore::Input::GetAxis(MetaCore::Input::Controllers::Left,  MetaCore::Input::Axes::ThumbstickVertical);
        leftTriggerInput               = MetaCore::Input::GetAxis(MetaCore::Input::Controllers::Left,  MetaCore::Input::Axes::TriggerStrength);
        leftGripInput                  = MetaCore::Input::GetAxis(MetaCore::Input::Controllers::Left,  MetaCore::Input::Axes::GripStrength);
        rightThumbstickHorizontalInput = MetaCore::Input::GetAxis(MetaCore::Input::Controllers::Right, MetaCore::Input::Axes::ThumbstickHorizontal);
        rightThumbstickVerticalInput   = MetaCore::Input::GetAxis(MetaCore::Input::Controllers::Right, MetaCore::Input::Axes::ThumbstickVertical);
        rightTriggerInput              = MetaCore::Input::GetAxis(MetaCore::Input::Controllers::Right, MetaCore::Input::Axes::TriggerStrength);
        rightGripInput                 = MetaCore::Input::GetAxis(MetaCore::Input::Controllers::Right, MetaCore::Input::Axes::GripStrength);
    }
};
ControllerState controllerState;
void handleControllerInputs() {
    ControllerState oldControllerState = controllerState;
    controllerState.update();

    if(!oldControllerState.isButtonXHeld         && controllerState.isButtonXHeld)         ControllerFunctions::invoke(ControllerFunctions::clickFunctions, ControllerCallbacks::buttonXOnClick);
    if(!oldControllerState.isButtonYHeld         && controllerState.isButtonYHeld)         ControllerFunctions::invoke(ControllerFunctions::clickFunctions, ControllerCallbacks::buttonYOnClick);
    if(!oldControllerState.isLeftThumbstickHeld  && controllerState.isLeftThumbstickHeld)  ControllerFunctions::invoke(ControllerFunctions::clickFunctions, ControllerCallbacks::leftThumbstickOnClick);
    if(!oldControllerState.isLeftTriggerHeld     && controllerState.isLeftTriggerHeld)     ControllerFunctions::invoke(ControllerFunctions::clickFunctions, ControllerCallbacks::leftTriggerOnClick);
    if(!oldControllerState.isLeftGripHeld        && controllerState.isLeftGripHeld)        ControllerFunctions::invoke(ControllerFunctions::clickFunctions, ControllerCallbacks::leftGripOnClick);
    if(!oldControllerState.isButtonAHeld         && controllerState.isButtonAHeld)         ControllerFunctions::invoke(ControllerFunctions::clickFunctions, ControllerCallbacks::buttonAOnClick);
    if(!oldControllerState.isButtonBHeld         && controllerState.isButtonBHeld)         ControllerFunctions::invoke(ControllerFunctions::clickFunctions, ControllerCallbacks::buttonBOnClick);
    if(!oldControllerState.isRightThumbstickHeld && controllerState.isRightThumbstickHeld) ControllerFunctions::invoke(ControllerFunctions::clickFunctions, ControllerCallbacks::rightThumbstickOnClick);
    if(!oldControllerState.isRightTriggerHeld    && controllerState.isRightTriggerHeld)    ControllerFunctions::invoke(ControllerFunctions::clickFunctions, ControllerCallbacks::rightTriggerOnClick);
    if(!oldControllerState.isRightGripHeld       && controllerState.isRightGripHeld)       ControllerFunctions::invoke(ControllerFunctions::clickFunctions, ControllerCallbacks::rightGripOnClick);

    ControllerFunctions::invoke<float>(ControllerFunctions::holdFunctions, ControllerCallbacks::buttonXOnHold,         controllerState.isButtonXHeld);
    ControllerFunctions::invoke<float>(ControllerFunctions::holdFunctions, ControllerCallbacks::buttonYOnHold,         controllerState.isButtonYHeld);
    ControllerFunctions::invoke<float>(ControllerFunctions::holdFunctions, ControllerCallbacks::leftThumbstickOnHold,  controllerState.isLeftThumbstickHeld);
    ControllerFunctions::invoke<float>(ControllerFunctions::holdFunctions, ControllerCallbacks::buttonAOnHold,         controllerState.isButtonAHeld);
    ControllerFunctions::invoke<float>(ControllerFunctions::holdFunctions, ControllerCallbacks::buttonBOnHold,         controllerState.isButtonBHeld);
    ControllerFunctions::invoke<float>(ControllerFunctions::holdFunctions, ControllerCallbacks::rightThumbstickOnHold, controllerState.isRightThumbstickHeld);

    ControllerFunctions::invoke<float>(ControllerFunctions::holdFunctions, ControllerCallbacks::leftTriggerOnHold,   controllerState.leftTriggerInput);
    ControllerFunctions::invoke<float>(ControllerFunctions::holdFunctions, ControllerCallbacks::leftGripOnHold,      controllerState.leftGripInput);
    ControllerFunctions::invoke<float>(ControllerFunctions::holdFunctions, ControllerCallbacks::rightTriggerOnHold,  controllerState.rightTriggerInput);
    ControllerFunctions::invoke<float>(ControllerFunctions::holdFunctions, ControllerCallbacks::rightGripOnHold,     controllerState.rightGripInput);

    ControllerFunctions::invoke<float>(ControllerFunctions::thumbstickFunctions, ControllerCallbacks::leftThumbstickHorizontalOnUpdate,  controllerState.leftThumbstickHorizontalInput);
    ControllerFunctions::invoke<float>(ControllerFunctions::thumbstickFunctions, ControllerCallbacks::leftThumbstickVerticalOnUpdate,    controllerState.leftThumbstickVerticalInput);
    ControllerFunctions::invoke<float>(ControllerFunctions::thumbstickFunctions, ControllerCallbacks::rightThumbstickHorizontalOnUpdate, controllerState.rightThumbstickHorizontalInput);
    ControllerFunctions::invoke<float>(ControllerFunctions::thumbstickFunctions, ControllerCallbacks::rightThumbstickVerticalOnUpdate,   controllerState.rightThumbstickVerticalInput);
}

void handleMapStart() {
    initReferences();
    isPracticing = true; // TODO Detect if in practice or normal mode
}

void handleMapUpdate() {
    if(!areShortcutsEnabled()) return;
    handleControllerInputs();
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

    PaperLogger.info("Installing hooks...");

    INSTALL_HOOK(PaperLogger, NoteCutSoundEffectManager_HandleNoteWasSpawned);
    INSTALL_HOOK(PaperLogger, BeatmapCallbacksController_ManualUpdate);

    PaperLogger.info("Installed all hooks!");
}