#include "controllerFunctions.hpp"
#include "main.hpp"
#include "config.hpp"

#include "game.hpp"

#include "GlobalNamespace/AudioTimeSyncController.hpp"
#include "UnityEngine/Time.hpp"

namespace PracticeTools::ControllerFunctions {

    static std::set<float> markers = {};

    void clearMarkers() {
        markers.clear();
    }

    void execute(ClickFunction function) {
        switch(function) {
            case ClickFunction::None:
                break;

            case ClickFunction::SkipForward: {
                float offset = getPracticeToolsConfig().skipDuration.GetValue();
                Game::manualSeekToAbsolute(Game::audioTimeSyncController->songTime + offset);
                break;
            }
            case ClickFunction::SkipBackward: {
                float offset = -getPracticeToolsConfig().skipDuration.GetValue();
                Game::manualSeekToAbsolute(Game::audioTimeSyncController->songTime + offset);
                break;
            }
            case ClickFunction::IncreaseSpeed: {
                float scaleFactor = 1 + getPracticeToolsConfig().speedChangePercentage.GetValue() / 100;
                Game::manualSetTimeScale(Game::audioTimeSyncController->timeScale * scaleFactor);
                break;
            }
            case ClickFunction::DecreaseSpeed: {
                float scaleFactor = 1 + getPracticeToolsConfig().speedChangePercentage.GetValue() / 100;
                Game::manualSetTimeScale(Game::audioTimeSyncController->timeScale / scaleFactor);
                break;
            }
            case ClickFunction::ResetSpeed: {
                Game::manualSetTimeScale(1);
                break;
            }
            case ClickFunction::PausePlay: {
                if(Game::audioTimeSyncController->state == GlobalNamespace::AudioTimeSyncController::State::Playing)
                    Game::manualPause();
                else Game::manualResume();
                break;
            }
            case ClickFunction::AddMarker: {
                markers.emplace(Game::audioTimeSyncController->songTime);
                break;
            }
            case ClickFunction::RemoveMarker: {
                auto nextMarker = markers.upper_bound(Game::audioTimeSyncController->songTime);
                if (nextMarker != markers.begin())
                    markers.erase(std::prev(nextMarker));
                break;
            }
            case ClickFunction::NextMarker: {
                auto nextMarker = markers.upper_bound(Game::audioTimeSyncController->songTime);
                if (!markers.empty() && nextMarker != markers.end())
                    Game::manualSeekToAbsolute(*nextMarker);
                break;
            }
            case ClickFunction::PreviousMarker: {
                float doubleClickSeconds = 0.5;
                auto nextMarker = markers.lower_bound(Game::audioTimeSyncController->songTime - doubleClickSeconds);
                if (nextMarker == markers.begin())
                    Game::manualSeekToAbsolute(0);
                else
                    Game::manualSeekToAbsolute(*std::prev(nextMarker));
                break;
            }

            default:
                PaperLogger.error("Unrecognized ClickFunction: {}", static_cast<int>(function));
        }
    }

    void execute(HoldFunction function, float input) {
        switch(function) {
            case HoldFunction::None:
                break;

            case HoldFunction::SkipForward: {
                float offset = getPracticeToolsConfig().skipDuration.GetValue() * input * getPracticeToolsConfig().sensitivity.GetValue() * UnityEngine::Time::get_deltaTime();
                if (Game::audioTimeSyncController->state == GlobalNamespace::AudioTimeSyncController::State::Playing)
                    offset -= UnityEngine::Time::get_deltaTime();
                Game::manualSeekToAbsolute(Game::audioTimeSyncController->songTime + offset);
                break;
            }
            case HoldFunction::SkipBackward: {
                float offset = -getPracticeToolsConfig().skipDuration.GetValue() * input * getPracticeToolsConfig().sensitivity.GetValue() * UnityEngine::Time::get_deltaTime();
                if (Game::audioTimeSyncController->state == GlobalNamespace::AudioTimeSyncController::State::Playing)
                    offset -= UnityEngine::Time::get_deltaTime();
                Game::manualSeekToAbsolute(Game::audioTimeSyncController->songTime + offset);
                break;
            }
            case HoldFunction::IncreaseSpeed: {
                float scaleFactor = pow(1 + getPracticeToolsConfig().speedChangePercentage.GetValue() / 100 * input, UnityEngine::Time::get_deltaTime());
                Game::manualSetTimeScale(Game::audioTimeSyncController->timeScale * scaleFactor);
                break;
            }
            case HoldFunction::DecreaseSpeed: {
                float scaleFactor = pow(1 + getPracticeToolsConfig().speedChangePercentage.GetValue() / 100 * input, UnityEngine::Time::get_deltaTime());
                Game::manualSetTimeScale(Game::audioTimeSyncController->timeScale / scaleFactor);
                break;
            }

            default:
                PaperLogger.error("Unrecognized HoldFunction: {}", static_cast<int>(function));
        }
    }

    void execute(ThumbstickFunction function, float input) {
        switch(function) {
            case ThumbstickFunction::None:
                break;

            case ThumbstickFunction::TimeSkip: {
                float offset = getPracticeToolsConfig().skipDuration.GetValue() * input * getPracticeToolsConfig().sensitivity.GetValue() * UnityEngine::Time::get_deltaTime();
                if (Game::audioTimeSyncController->state == GlobalNamespace::AudioTimeSyncController::State::Playing)
                    offset -= UnityEngine::Time::get_deltaTime();
                Game::manualSeekToAbsolute(Game::audioTimeSyncController->songTime + offset);
                break;
            }
            case ThumbstickFunction::AdjustSpeed: {
                float scaleFactor = pow(1 + getPracticeToolsConfig().speedChangePercentage.GetValue() / 100 * abs(input), UnityEngine::Time::get_deltaTime());
                if(input < 0) scaleFactor = 1 / scaleFactor;
                Game::manualSetTimeScale(Game::audioTimeSyncController->timeScale * scaleFactor);
                break;
            }

            default:
                PaperLogger.error("Unrecognized ThumbstickFunction: {}", static_cast<int>(function));
        }
    }

}