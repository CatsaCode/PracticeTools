#include "controllerFunctions.hpp"
#include "main.hpp"
#include "config.hpp"

#include "game.hpp"

#include "GlobalNamespace/AudioTimeSyncController.hpp"
#include "UnityEngine/Time.hpp"

// TODO Implement all the controller callback functions
namespace PracticeTools::ControllerFunctions {

    FunctionList<void()> clickFunctions = {
        {"Skip Forward", [](){
            float offset = getPracticeToolsConfig().skipDuration.GetValue();
            Game::manualSeekToAbsolute(Game::audioTimeSyncController->get_songTime() + offset);
        }},
        {"Skip Backward", [](){
            float offset = -getPracticeToolsConfig().skipDuration.GetValue();
            Game::manualSeekToAbsolute(Game::audioTimeSyncController->get_songTime() + offset);
        }},
        {"Increase Speed", [](){
            float scaleFactor = 1 + getPracticeToolsConfig().speedChangePercentage.GetValue() / 100;
            Game::manualSetTimeScale(Game::audioTimeSyncController->get_timeScale() * scaleFactor);
        }},
        {"Decrease Speed", [](){
            float scaleFactor = 1 + getPracticeToolsConfig().speedChangePercentage.GetValue() / 100;
            Game::manualSetTimeScale(Game::audioTimeSyncController->get_timeScale() / scaleFactor);
        }},
        {"Reset Speed", [](){
            Game::manualSetTimeScale(1);
        }},
        {"Pause / Play", [](){
            if(Game::audioTimeSyncController->get_state() == GlobalNamespace::AudioTimeSyncController::State::Playing)
                Game::manualPause();
            else Game::manualResume();
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
            float offset = getPracticeToolsConfig().skipDuration.GetValue() * input * getPracticeToolsConfig().sensitivity.GetValue() * UnityEngine::Time::get_deltaTime();
            Game::manualSeekToAbsolute(Game::audioTimeSyncController->get_songTime() + offset);
        }},
        {"Skip Backward", [](float input){
            float offset = -getPracticeToolsConfig().skipDuration.GetValue() * input * getPracticeToolsConfig().sensitivity.GetValue() * UnityEngine::Time::get_deltaTime();
            Game::manualSeekToAbsolute(Game::audioTimeSyncController->get_songTime() + offset);
        }},
        {"Increase Speed", [](float input){
            float scaleFactor = pow(1 + getPracticeToolsConfig().speedChangePercentage.GetValue() / 100 * input, UnityEngine::Time::get_deltaTime());
            Game::manualSetTimeScale(Game::audioTimeSyncController->get_timeScale() * scaleFactor);
        }},
        {"Decrease Speed", [](float input){
            float scaleFactor = pow(1 + getPracticeToolsConfig().speedChangePercentage.GetValue() / 100 * input, UnityEngine::Time::get_deltaTime());
            Game::manualSetTimeScale(Game::audioTimeSyncController->get_timeScale() / scaleFactor);
        }}
    };

    FunctionList<void(float)> thumbstickFunctions = {
        {"Skip", [](float input){
            float offset = getPracticeToolsConfig().skipDuration.GetValue() * input * getPracticeToolsConfig().sensitivity.GetValue() * UnityEngine::Time::get_deltaTime();
            Game::manualSeekToAbsolute(Game::audioTimeSyncController->get_songTime() + offset);
        }},
        {"Speed", [](float input){
            float scaleFactor = pow(1 + getPracticeToolsConfig().speedChangePercentage.GetValue() / 100 * abs(input), UnityEngine::Time::get_deltaTime());
            if(input < 0) scaleFactor = 1 / scaleFactor;
            Game::manualSetTimeScale(Game::audioTimeSyncController->get_timeScale() * scaleFactor);
        }}
    };

    void invoke(FunctionList<void()>& list, std::string_view name) {
        if(name.empty()) return;
        if(!list.contains(name)) {PaperLogger.error("Could not invoke function: '{}'", name); return;}
        list[name]();
    }

}