#include "game.hpp"
#include "main.hpp"
#include "config.hpp"

#include "metacore/shared/internals.hpp"
#include "metacore/shared/game.hpp"

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
#include "GlobalNamespace/AudioManagerSO.hpp"
#include "GlobalNamespace/BeatmapDataItem.hpp"
#include "GlobalNamespace/NoteData.hpp"
#include "GlobalNamespace/NoteController.hpp"
#include "GlobalNamespace/GameplayCoreInstaller.hpp"
#include "UnityEngine/Resources.hpp"
#include "UnityEngine/GameObject.hpp"
#include "System/Collections/Generic/Dictionary_2.hpp"
#include "System/Collections/Generic/LinkedList_1.hpp"
#include "System/Collections/Generic/LinkedListNode_1.hpp"
#include "UnityEngine/AudioSource.hpp"
#include "UnityEngine/AudioClip.hpp"

namespace PracticeTools::Game {

    UnityW<GlobalNamespace::AudioTimeSyncController> audioTimeSyncController = nullptr;

    static UnityW<GlobalNamespace::PauseController> pauseController = nullptr;
    static UnityW<GlobalNamespace::NoteCutSoundEffectManager> noteCutSoundEffectManager = nullptr;
    static UnityW<GlobalNamespace::AudioManagerSO> audioManager = nullptr;
    static GlobalNamespace::IGamePause* gamePause = nullptr;
    static GlobalNamespace::BeatmapObjectManager* beatmapObjectManager = nullptr;
    static GlobalNamespace::BeatmapCallbacksController* beatmapCallbacksController = nullptr;

    static bool isPracticing = false;
    static bool areReferencesSafe = false;
    static bool isTimeSkipping = false;

    bool areShortcutsEnabled() {
        return getPracticeToolsConfig().enabled.GetValue() && isPracticing && areReferencesSafe;
    }

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

    void resetReferences() {
        areReferencesSafe = false;
        isPracticing = false;
        isTimeSkipping = false;
    }



    MAKE_HOOK_MATCH(GameplayCoreInstaller_InstallBindings, &GlobalNamespace::GameplayCoreInstaller::InstallBindings,
        void, GlobalNamespace::GameplayCoreInstaller* self
    ) {
        GameplayCoreInstaller_InstallBindings(self);
        isPracticing = self->_sceneSetupData->___practiceSettings != nullptr; // ->practiceSettings not defined
        MetaCore::Game::SetScoreSubmission(MOD_ID, !(getPracticeToolsConfig().enabled.GetValue() && isPracticing));
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

    void installHooks() {
        INSTALL_HOOK(PaperLogger, GameplayCoreInstaller_InstallBindings);
        INSTALL_HOOK(PaperLogger, NoteCutSoundEffectManager_HandleNoteWasSpawned);
        INSTALL_HOOK(PaperLogger, BeatmapCallbacksController_ManualUpdate);
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
        if(getPracticeToolsConfig().fullPause.GetValue()) gamePause->Pause();
        else audioTimeSyncController->Pause();
        stopAllNoteCutSoundEffects();
    }

    void manualResume() {
        if(getPracticeToolsConfig().fullPause.GetValue()) gamePause->Resume();
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
}