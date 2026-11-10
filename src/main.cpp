#include "main.hpp"
#include "config.hpp"

#include "controllerInput.hpp"
#include "game.hpp"

#include "scotland2/shared/modloader.h"

#include "metacore/shared/events.hpp"
#include "metacore/shared/internals.hpp"


#include <cstdlib>
#include <unordered_map>

static modloader::ModInfo modInfo{MOD_ID, VERSION, 0};
// Stores the ID and version of our mod, and is sent to
// the modloader upon startup

void handleMapStart() {
    PracticeTools::Game::initReferences();
}

void handleMapUpdate() {
    if(!PracticeTools::Game::areShortcutsEnabled())
        return;

    PracticeTools::ControllerInput::handleControllerInputs();
}

void handleMapEnd() {
    PracticeTools::Game::resetReferences();
}


// Called at the early stages of game loading
MOD_EXTERN_FUNC void setup(CModInfo *info) noexcept {
    *info = modInfo.to_c();

    // File logging
    Paper::Logger::RegisterFileContextId(PaperLogger.tag);

    PaperLogger.info("Completed setup!");
}

// Called later on in the game loading - a good time to install function hooks
MOD_EXTERN_FUNC void late_load() noexcept {
    il2cpp_functions::Init();

    getPracticeToolsConfig().Init(modInfo);

    MetaCore::Events::AddCallback(MetaCore::Events::MapStarted, handleMapStart);
    MetaCore::Events::AddCallback(MetaCore::Events::Update, handleMapUpdate);
    MetaCore::Events::AddCallback(MetaCore::Events::MapEnded, handleMapEnd);

    PaperLogger.info("Installing hooks...");

    PracticeTools::Game::installGameHooks();

    PaperLogger.info("Installed all hooks!");
}