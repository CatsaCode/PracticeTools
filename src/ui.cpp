#include "ui.hpp"
#include "main.hpp"
#include "config.hpp"

#include "controllerFunctions.hpp"

#include "bsml/shared/BSML.hpp"
#include "bsml/shared/Helpers/creation.hpp"

#include "GlobalNamespace/SinglePlayerLevelSelectionFlowCoordinator.hpp"

using namespace GlobalNamespace;

DEFINE_TYPE(PracticeTools::UI, PracticeToolsViewController);

namespace PracticeTools::UI {
    static UnityW<PracticeToolsViewController> practiceToolsViewController = nullptr;

    MAKE_HOOK_MATCH(
        SinglePlayerLevelSelectionFlowCoordinator_LevelSelectionFlowCoordinatorTopViewControllerWillChange,
        &SinglePlayerLevelSelectionFlowCoordinator::LevelSelectionFlowCoordinatorTopViewControllerWillChange,
        void, SinglePlayerLevelSelectionFlowCoordinator* self,
        HMUI::ViewController* oldViewController, HMUI::ViewController* newViewController,
        HMUI::ViewController::AnimationType animationType
    ) {
        SinglePlayerLevelSelectionFlowCoordinator_LevelSelectionFlowCoordinatorTopViewControllerWillChange(self, oldViewController, newViewController, animationType);

        if (!practiceToolsViewController)
            practiceToolsViewController = BSML::Helpers::CreateViewController<PracticeToolsViewController*>();

        if (newViewController == self->_practiceViewController.unsafePtr() &&
            oldViewController != self->_practiceViewController.unsafePtr())
        {
            self->SetRightScreenViewController(practiceToolsViewController, animationType);

            // Not sure why the leaderboard only partly disappears. Ah oh well, this works
            if (self->leaderboardViewController)
                self->leaderboardViewController->gameObject->SetActive(false);
        }
    }

    // Prevent rightScreenViewController = beatmapKey ? leaderboardViewController : nullptr
    MAKE_HOOK_MATCH(
        SinglePlayerLevelSelectionFlowCoordinator_SelectionDidChange,
        &SinglePlayerLevelSelectionFlowCoordinator::SelectionDidChange,
        void, SinglePlayerLevelSelectionFlowCoordinator* self,
        BeatmapLevelPack* pack, ByRef<BeatmapKey> beatmapKey
    ) {
        if (self->rightScreenViewController == practiceToolsViewController.unsafePtr() &&
            self->rightScreenViewController != nullptr)
            return;

        SinglePlayerLevelSelectionFlowCoordinator_SelectionDidChange(self, pack, beatmapKey);
    }

    void installHooks() {
        INSTALL_HOOK(PaperLogger, SinglePlayerLevelSelectionFlowCoordinator_LevelSelectionFlowCoordinatorTopViewControllerWillChange);
        INSTALL_HOOK(PaperLogger, SinglePlayerLevelSelectionFlowCoordinator_SelectionDidChange);
    }



    int findDropdownIndex(std::span<const std::string_view> values, StringW value) {
        return std::distance(values.begin(), std::find(values.begin(), values.end(), value));
    }

    void PracticeToolsViewController::DidActivate(bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
        if (!firstActivation)
            return;

        auto* container = BSML::Lite::CreateScrollableSettingsContainer(transform);

        using namespace ControllerFunctions;
        #define CREATE_BINDING_DROPDOWN(parent, label, values, configName)  \
        BSML::Lite::CreateDropdown(parent, label,                           \
            values[getPracticeToolsConfig().configName.GetValue()], values, \
            [](StringW value){getPracticeToolsConfig().configName.SetValue( \
                findDropdownIndex(values, value));})                        \

        CREATE_BINDING_DROPDOWN(container, "Click X", clickFunctionLabels, buttonXOnClick);
        CREATE_BINDING_DROPDOWN(container, "Hold X", holdFunctionLabels, buttonXOnHold);
        CREATE_BINDING_DROPDOWN(container, "Click Y", clickFunctionLabels, buttonYOnClick);
        CREATE_BINDING_DROPDOWN(container, "Hold Y", holdFunctionLabels, buttonYOnHold);
        CREATE_BINDING_DROPDOWN(container, "Click Left Trigger", clickFunctionLabels, leftTriggerOnClick);
        CREATE_BINDING_DROPDOWN(container, "Hold Left Trigger", holdFunctionLabels, leftTriggerOnHold);
        CREATE_BINDING_DROPDOWN(container, "Click Left Grip", clickFunctionLabels, leftGripOnClick);
        CREATE_BINDING_DROPDOWN(container, "Hold Left Grip", holdFunctionLabels, leftGripOnHold);
        CREATE_BINDING_DROPDOWN(container, "Click Left Thumbstick", clickFunctionLabels, leftThumbstickOnClick);
        CREATE_BINDING_DROPDOWN(container, "Hold Left Thumbstick", holdFunctionLabels, leftThumbstickOnHold);
        CREATE_BINDING_DROPDOWN(container, "Horizontal Left Thumbstick", thumbstickFunctionLabels, leftThumbstickHorizontalOnUpdate);
        CREATE_BINDING_DROPDOWN(container, "Vertical Left Thumbstick", thumbstickFunctionLabels, leftThumbstickVerticalOnUpdate);

        CREATE_BINDING_DROPDOWN(container, "Click A", clickFunctionLabels, buttonAOnClick);
        CREATE_BINDING_DROPDOWN(container, "Hold A", holdFunctionLabels, buttonAOnHold);
        CREATE_BINDING_DROPDOWN(container, "Click B", clickFunctionLabels, buttonBOnClick);
        CREATE_BINDING_DROPDOWN(container, "Hold B", holdFunctionLabels, buttonBOnHold);
        CREATE_BINDING_DROPDOWN(container, "Click Right Trigger", clickFunctionLabels, rightTriggerOnClick);
        CREATE_BINDING_DROPDOWN(container, "Hold Right Trigger", holdFunctionLabels, rightTriggerOnHold);
        CREATE_BINDING_DROPDOWN(container, "Click Right Grip", clickFunctionLabels, rightGripOnClick);
        CREATE_BINDING_DROPDOWN(container, "Hold Right Grip", holdFunctionLabels, rightGripOnHold);
        CREATE_BINDING_DROPDOWN(container, "Click Right Thumbstick", clickFunctionLabels, rightThumbstickOnClick);
        CREATE_BINDING_DROPDOWN(container, "Hold Right Thumbstick", holdFunctionLabels, rightThumbstickOnHold);
        CREATE_BINDING_DROPDOWN(container, "Horizontal Right Thumbstick", thumbstickFunctionLabels, rightThumbstickHorizontalOnUpdate);
        CREATE_BINDING_DROPDOWN(container, "Vertical Right Thumbstick", thumbstickFunctionLabels, rightThumbstickVerticalOnUpdate);
    }
}