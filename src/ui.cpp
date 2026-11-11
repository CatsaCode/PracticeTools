#include "ui.hpp"
#include "main.hpp"
#include "config.hpp"

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



    void PracticeToolsViewController::DidActivate(bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
        if (!firstActivation)
            return;

        auto* container = BSML::Lite::CreateScrollableSettingsContainer(transform);

        BSML::Lite::CreateUIButton(container, "Test", [](){
            PaperLogger.debug("This sure is button!");
        });
    }
}