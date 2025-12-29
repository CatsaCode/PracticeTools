#include "main.hpp"

#include "scotland2/shared/modloader.h"

#include "metacore/shared/input.hpp"
#include "metacore/shared/events.hpp"
#include "metacore/shared/internals.hpp"
#include "GlobalNamespace/AudioTimeSyncController.hpp"
#include "GlobalNamespace/GameSongController.hpp"
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

void handleAOnPress() {
    if(!MetaCore::Input::GetPressed(MetaCore::Input::Controllers::Right, MetaCore::Input::Buttons::AX)) return;
    PaperLogger.debug("A Pressed");
    
    auto audioTimeSyncController = MetaCore::Internals::audioTimeSyncController;
    if(!audioTimeSyncController) {PaperLogger.error("Could not get AudioTimeSyncController"); return;}

    auto beatmapObjectManager = MetaCore::Internals::beatmapObjectManager;
    if(!beatmapObjectManager) {PaperLogger.error("Could not get BeatmapObjectManager"); return;}

    auto beatmapCallbacksController = MetaCore::Internals::beatmapCallbacksController;
    if(!beatmapCallbacksController) {PaperLogger.error("Could not get BeatmapCallbacksController"); return;}

    auto noteCutSoundEffectManager = UnityEngine::Object::FindObjectOfType<GlobalNamespace::NoteCutSoundEffectManager*>();
    if(!noteCutSoundEffectManager) {PaperLogger.error("Could not get NoteCutSoundEffectManager"); return;}


    if(!MetaCore::Input::GetPressed(MetaCore::Input::Controllers::Left, MetaCore::Input::Buttons::AX)) {
        audioTimeSyncController->Pause();
    } else {
        float offset = -1;
        audioTimeSyncController->StartSong(audioTimeSyncController->get_songTime() - audioTimeSyncController->get_startSongTime() + offset);

        // Taken from ReLoader hehe
        for(int i = 0; i < beatmapObjectManager->_allBeatmapObjects->get_Count(); i++) {
            auto beatmapObject = beatmapObjectManager->_allBeatmapObjects->get_Item(i);
            reinterpret_cast<UnityEngine::MonoBehaviour*>(beatmapObject)->get_gameObject()->SetActive(true);
            beatmapObject->Dissolve(1);
        }

        for(int i = 0; i < noteCutSoundEffectManager->_noteCutSoundEffectPoolContainer->get_activeItems()->get_Count(); i++) {
            auto noteCutSoundEffect = noteCutSoundEffectManager->_noteCutSoundEffectPoolContainer->get_activeItems()->get_Item(i);
            noteCutSoundEffect->StopPlayingAndFinish();
        }
        noteCutSoundEffectManager->_prevNoteATime = 0;
        noteCutSoundEffectManager->_prevNoteBTime = 0;

        for(auto callbacksAtTime : beatmapCallbacksController->_callbacksInTimes->_entries) {
            callbacksAtTime.value->lastProcessedNode = nullptr;
        }
        beatmapCallbacksController->_prevSongTime = 0;
    }
}

void handleBOnPress() {
    if(!MetaCore::Input::GetPressed(MetaCore::Input::Controllers::Right, MetaCore::Input::Buttons::BY)) return;
    PaperLogger.debug("B Pressed");
    PaperLogger.debug("# AudioTimeSyncControllers: {}", UnityEngine::Resources::FindObjectsOfTypeAll<GlobalNamespace::AudioTimeSyncController*>().size());
    auto audioTimeSyncController = UnityEngine::Resources::FindObjectsOfTypeAll<GlobalNamespace::AudioTimeSyncController*>()->FirstOrDefault();
    if(!audioTimeSyncController) return;

    audioTimeSyncController->Resume();
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