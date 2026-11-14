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
#include "GlobalNamespace/GameplayCoreInstaller.hpp"
#include "UnityEngine/Resources.hpp"
#include "UnityEngine/GameObject.hpp"
#include "System/Collections/Generic/Dictionary_2.hpp"
#include "System/Collections/Generic/LinkedListNode_1.hpp"
#include "UnityEngine/AudioSource.hpp"
#include "UnityEngine/AudioClip.hpp"
#include "UnityEngine/Time.hpp"

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

    bool areShortcutsEnabled() {
        return getPracticeToolsConfig().enabled.GetValue() && isPracticing && areReferencesSafe;
    }

    bool initReferences() {
        areReferencesSafe = false;

        #define FIND_OBJECT(name, source)                 \
        name = source;                                    \
        if (!name) {                                      \
            PaperLogger.error("Could not find " #name);   \
            return false;                                 \
        }

        FIND_OBJECT(audioTimeSyncController, UnityEngine::Object::FindObjectOfType<GlobalNamespace::AudioTimeSyncController*>());
        FIND_OBJECT(pauseController, UnityEngine::Object::FindObjectOfType<GlobalNamespace::PauseController*>());
        FIND_OBJECT(noteCutSoundEffectManager, UnityEngine::Object::FindObjectOfType<GlobalNamespace::NoteCutSoundEffectManager*>());
        FIND_OBJECT(gamePause, pauseController->_gamePause);
        FIND_OBJECT(audioManager, noteCutSoundEffectManager->_audioManager);
        FIND_OBJECT(beatmapObjectManager, MetaCore::Internals::beatmapObjectManager);
        FIND_OBJECT(beatmapCallbacksController, MetaCore::Internals::beatmapCallbacksController);

        #undef FIND_OBJECT

        areReferencesSafe = true;
        return true;
    }

    void resetReferences() {
        areReferencesSafe = false;
        isPracticing = false;
    }



    MAKE_HOOK_MATCH(
        GameplayCoreInstaller_InstallBindings,
        &GlobalNamespace::GameplayCoreInstaller::InstallBindings,
        void, GlobalNamespace::GameplayCoreInstaller* self
    ) {
        GameplayCoreInstaller_InstallBindings(self);
        isPracticing = self->_sceneSetupData->___practiceSettings != nullptr; // ->practiceSettings not defined
        MetaCore::Game::SetScoreSubmission(MOD_ID, !(getPracticeToolsConfig().enabled.GetValue() && isPracticing));
    }

    void installHooks() {
        INSTALL_HOOK(PaperLogger, GameplayCoreInstaller_InstallBindings);
    }



    void stopAllNoteCutSoundEffects() {
        for (int i = 0; i < noteCutSoundEffectManager->_noteCutSoundEffectPoolContainer->activeItems->Count; i++) {
            auto noteCutSoundEffect = noteCutSoundEffectManager->_noteCutSoundEffectPoolContainer->activeItems->get_Item(i);
            noteCutSoundEffect->StopPlayingAndFinish();
        }
    }

    void despawnAllObjects() {
        for (int i = 0; i < beatmapObjectManager->_allBeatmapObjects->Count; i++) {
            auto beatmapObject = beatmapObjectManager->_allBeatmapObjects->get_Item(i);
            // Dissolve instead of disable to trigger full despawn
            reinterpret_cast<UnityEngine::MonoBehaviour*>(beatmapObject)->gameObject->SetActive(true);
            beatmapObject->Dissolve(0);
        }
    }



    void manualPause() {
        if (getPracticeToolsConfig().fullPause.GetValue())
            gamePause->Pause();
        else
            audioTimeSyncController->Pause();

        stopAllNoteCutSoundEffects();
    }

    void manualResume() {
        if (getPracticeToolsConfig().fullPause.GetValue())
            gamePause->Resume();
        else
            audioTimeSyncController->Resume();
    }

    void manualSeekToAbsolute(float songTime) {
        float deltaSongTime = songTime - audioTimeSyncController->songTime;

        songTime = std::max(0.0f, std::min(songTime, audioTimeSyncController->_audioSource->clip->length - 0.01f));
        audioTimeSyncController->_startSongTime = songTime;
        audioTimeSyncController->SeekTo(0);

        beatmapCallbacksController->_songTime = songTime;
        // Letting Beat Saber invoke callbacks in reverse can crash the game
        beatmapCallbacksController->_prevSongTime = songTime - UnityEngine::Time::get_deltaTime();

        stopAllNoteCutSoundEffects();
        noteCutSoundEffectManager->_prevNoteATime = songTime;
        noteCutSoundEffectManager->_prevNoteBTime = songTime;

        if (deltaSongTime >= 0)
            return;

        for (auto callbacksInTime : beatmapCallbacksController->_callbacksInTimes->_entries) {
            auto* value = callbacksInTime.value;
            auto* node = value->lastProcessedNode;

            // Reverse lastProcessedNode to the new spawn time
            while (
                node &&
                node->Value->time - value->aheadTime > songTime
            ) {
                if (node->Value->type == GlobalNamespace::BeatmapDataItem::BeatmapDataItemType::BeatmapEvent)
                    beatmapCallbacksController->_callCallbacksBehavior->CallCallbacks(value, node->Value);

                value->lastProcessedNode = node->Previous;
                node = node->Previous;
            }

            // Spawn nearby notes that haven't been spawned yet
            while (
                node &&
                node->Value->time > songTime
            ) {
                if (
                    node->Value->type == GlobalNamespace::BeatmapDataItem::BeatmapDataItemType::BeatmapObject &&
                    node->Value->time <= songTime - deltaSongTime
                ) {
                    beatmapCallbacksController->_callCallbacksBehavior->CallCallbacks(value, node->Value);
                }

                node = node->Previous;
            }
        }
    }

    // Will likely need to hook and rewrite AudioTimeSyncController::Update to remove any time fixing stuff if I want to try backwards audio
    // I will also need a component or a more active hook to deal with the BeatmapCallbacksController callbacksInTimes
    void manualSetTimeScale(float timeScale) {
        audioTimeSyncController->_timeScale = timeScale;
        audioTimeSyncController->_audioSource->pitch = timeScale;
        audioManager->musicPitch = 1 / timeScale;
        // TODO Adjust speed of NoteCutSoundEffects, or just stop them
    }
}