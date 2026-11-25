#include "game.hpp"
#include "main.hpp"
#include "config.hpp"

#include "metacore/shared/internals.hpp"
#include "metacore/shared/game.hpp"

#include "GlobalNamespace/AudioTimeSyncController.hpp"
#include "GlobalNamespace/GameSongController.hpp"
#include "GlobalNamespace/PauseController.hpp"
#include "GlobalNamespace/IGamePause.hpp"
#include "GlobalNamespace/BasicBeatmapObjectManager.hpp"
#include "GlobalNamespace/BeatmapCallbacksController.hpp"
#include "GlobalNamespace/NoteCutSoundEffectManager.hpp"
#include "GlobalNamespace/NoteCutSoundEffect.hpp"
#include "GlobalNamespace/MemoryPoolContainer_1.hpp"
#include "GlobalNamespace/CallbacksInTime.hpp"
#include "GlobalNamespace/AudioManagerSO.hpp"
#include "GlobalNamespace/BeatmapDataItem.hpp"
#include "GlobalNamespace/BeatmapEventData.hpp"
#include "GlobalNamespace/BPMChangeBeatmapEventData.hpp"

#include "GlobalNamespace/IVariableMovementDataProvider.hpp"
#include "GlobalNamespace/NoteController.hpp"
#include "GlobalNamespace/NoteData.hpp"
#include "GlobalNamespace/NoteMovement.hpp"
#include "GlobalNamespace/NoteWaiting.hpp"
#include "GlobalNamespace/NoteJump.hpp"
#include "GlobalNamespace/SliderController.hpp"
#include "GlobalNamespace/SliderData.hpp"
#include "GlobalNamespace/SliderMovement.hpp"
#include "GlobalNamespace/ObstacleController.hpp"
#include "GlobalNamespace/ObstacleData.hpp"

#include "GlobalNamespace/GameplayCoreInstaller.hpp"

#include "UnityEngine/Resources.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/AudioSource.hpp"
#include "UnityEngine/AudioClip.hpp"
#include "UnityEngine/Time.hpp"

#include "System/Collections/Generic/Dictionary_2.hpp"
#include "System/Collections/Generic/LinkedListNode_1.hpp"

namespace PracticeTools::Game {

    UnityW<GlobalNamespace::AudioTimeSyncController> audioTimeSyncController = nullptr;

    static UnityW<GlobalNamespace::PauseController> pauseController = nullptr;
    static UnityW<GlobalNamespace::NoteCutSoundEffectManager> noteCutSoundEffectManager = nullptr;
    static UnityW<GlobalNamespace::AudioManagerSO> audioManager = nullptr;
    static GlobalNamespace::IGamePause* gamePause = nullptr;
    static GlobalNamespace::BasicBeatmapObjectManager* beatmapObjectManager = nullptr;
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
        FIND_OBJECT(beatmapObjectManager, reinterpret_cast<GlobalNamespace::BasicBeatmapObjectManager*>(MetaCore::Internals::beatmapObjectManager));
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

    // Direct BeatmapObjectManager::Despawn is prone to crashing
    void safeDespawn(GlobalNamespace::IBeatmapObjectController* beatmapObject) {
        beatmapObject->Hide(false);
        beatmapObject->Pause(false);
        il2cpp_utils::cast<UnityEngine::MonoBehaviour>(beatmapObject)->gameObject->SetActive(true);
        beatmapObject->Dissolve(0);
    }

    void despawnDidPassHalfBeatmapObjects() {
        for (int i = beatmapObjectManager->_allBeatmapObjects->Count - 1; i >= 0; i--) {
            auto* beatmapObject = beatmapObjectManager->_allBeatmapObjects->get_Item(i);

            if (auto noteController = il2cpp_utils::try_cast<GlobalNamespace::NoteController>(beatmapObject).value_or(nullptr))
                if (auto* noteData = noteController->_noteData)
                    if (audioTimeSyncController->songTime - noteData->time >= 0)
                        safeDespawn(beatmapObject);

            if (auto sliderController = il2cpp_utils::try_cast<GlobalNamespace::SliderController>(beatmapObject).value_or(nullptr))
                if (auto* sliderData = sliderController->_sliderData)
                    if (audioTimeSyncController->songTime - sliderData->time >= 0)
                        safeDespawn(beatmapObject);

            if (auto obstacleController = il2cpp_utils::try_cast<GlobalNamespace::ObstacleController>(beatmapObject).value_or(nullptr))
                if (auto* obstacleData = obstacleController->_obstacleData)
                    if (audioTimeSyncController->songTime - obstacleData->time >= 0)
                        safeDespawn(beatmapObject);
        }
    }

    void despawnPreWaitingBeatmapObjects() {
        float songTime = audioTimeSyncController->songTime;
        for (int i = beatmapObjectManager->_allBeatmapObjects->Count - 1; i >= 0; i--) {
            auto* beatmapObject = beatmapObjectManager->_allBeatmapObjects->get_Item(i);

            if (auto* noteController = il2cpp_utils::try_cast<GlobalNamespace::NoteController>(beatmapObject).value_or(nullptr)) {
                if (!noteController->_noteData || !noteController->_noteMovement || !noteController->_noteMovement->_variableMovementDataProvider)
                    continue;
                float noteTime = noteController->_noteData->time;
                float spawnAheadTime = noteController->_noteMovement->_variableMovementDataProvider->spawnAheadTime;
                if (songTime - noteTime < -spawnAheadTime)
                    safeDespawn(beatmapObject);
            }

            if (auto* sliderController = il2cpp_utils::try_cast<GlobalNamespace::SliderController>(beatmapObject).value_or(nullptr)) {
                if (!sliderController->_sliderData || !sliderController->_sliderMovement || !sliderController->_sliderMovement->_variableMovementDataProvider)
                    continue;
                float noteTime = sliderController->_sliderData->time;
                float spawnAheadTime = sliderController->_sliderMovement->_variableMovementDataProvider->spawnAheadTime;
                if (songTime - noteTime < -spawnAheadTime)
                    safeDespawn(beatmapObject);
            }

            if (auto* obstacleController = il2cpp_utils::try_cast<GlobalNamespace::ObstacleController>(beatmapObject).value_or(nullptr)) {
                if (!obstacleController->_obstacleData || !obstacleController->_variableMovementDataProvider)
                    continue;
                float noteTime = obstacleController->_obstacleData->time;
                float spawnAheadTime = obstacleController->_variableMovementDataProvider->spawnAheadTime;
                if (songTime - noteTime < -spawnAheadTime)
                    safeDespawn(beatmapObject);
            }
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
        audioTimeSyncController->_prevAudioSamplePos -= audioTimeSyncController->_audioSource->clip->frequency * UnityEngine::Time::get_deltaTime() * audioTimeSyncController->_timeScale;

        beatmapCallbacksController->_songTime = songTime;
        // Disable automatic callback reversal; BeatmapCallbacksController::ManualUpdate will crash if it attempts using default BPMChangeBeatmapEventData
        beatmapCallbacksController->_prevSongTime = songTime - UnityEngine::Time::get_deltaTime() * audioTimeSyncController->_timeScale;

        stopAllNoteCutSoundEffects();
        noteCutSoundEffectManager->_prevNoteATime = songTime;
        noteCutSoundEffectManager->_prevNoteBTime = songTime;

        if (deltaSongTime >= 0)
            return;

        despawnDidPassHalfBeatmapObjects();

        for (auto callbacksInTime : beatmapCallbacksController->_callbacksInTimes->_entries) {
            auto* value = callbacksInTime.value;
            if (!value)
                continue;
            auto* node = value->lastProcessedNode;

            while (
                node &&
                node->Value->time - value->aheadTime > songTime
            ) {
                if (node->Value->type == GlobalNamespace::BeatmapDataItem::BeatmapDataItemType::BeatmapEvent) {
                    auto beatmapEventData = il2cpp_utils::try_cast<GlobalNamespace::BeatmapEventData>(node->Value);
                    PaperLogger.debug("Executing previous {} callback, default: {}", il2cpp_utils::ClassStandardName(beatmapEventData.value()->klass), beatmapEventData.value()->previousSameTypeEventData == nullptr);
                    if (beatmapEventData.value()->previousSameTypeEventData)
                        beatmapCallbacksController->_callCallbacksBehavior->CallCallbacks(value, beatmapEventData.value()->previousSameTypeEventData);
                    else if (beatmapEventData.value()->GetDefault())
                        beatmapCallbacksController->_callCallbacksBehavior->CallCallbacks(value, beatmapEventData.value()->GetDefault(beatmapEventData.value()));
                }

                value->lastProcessedNode = node->Previous;
                node = node->Previous;
            }

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

        despawnPreWaitingBeatmapObjects();
    }

    // Will likely need to hook and rewrite AudioTimeSyncController::Update to remove any time fixing stuff if I want to try backwards audio
    // I will also need a component or a more active hook to deal with the BeatmapCallbacksController callbacksInTimes
    void manualSetTimeScale(float timeScale) {
        float expectedTime = UnityEngine::Time::get_timeSinceLevelLoad() * audioTimeSyncController->_timeScale - audioTimeSyncController->_audioStartTimeOffsetSinceStart;
        audioTimeSyncController->_audioStartTimeOffsetSinceStart = UnityEngine::Time::get_timeSinceLevelLoad() * timeScale - expectedTime;

        audioTimeSyncController->_timeScale = timeScale;
        audioTimeSyncController->_audioSource->pitch = timeScale;
        audioManager->musicPitch = 1 / timeScale;
        // TODO Adjust speed of NoteCutSoundEffects, or just stop them
    }
}