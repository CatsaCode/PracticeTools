#include "main.hpp"
#include "config.hpp"

#include "controllerFunctions.hpp"

#include "metacore/shared/input.hpp"

namespace PracticeTools::ControllerInput {

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

    static ControllerState controllerState;
    void handleControllerInputs() {
        ControllerState oldControllerState = controllerState;
        controllerState.update();

        if(!oldControllerState.isButtonXHeld         && controllerState.isButtonXHeld)         ControllerFunctions::execute(ControllerCallbacks::buttonXOnClick);
        if(!oldControllerState.isButtonYHeld         && controllerState.isButtonYHeld)         ControllerFunctions::execute(ControllerCallbacks::buttonYOnClick);
        if(!oldControllerState.isLeftThumbstickHeld  && controllerState.isLeftThumbstickHeld)  ControllerFunctions::execute(ControllerCallbacks::leftThumbstickOnClick);
        if(!oldControllerState.isLeftTriggerHeld     && controllerState.isLeftTriggerHeld)     ControllerFunctions::execute(ControllerCallbacks::leftTriggerOnClick);
        if(!oldControllerState.isLeftGripHeld        && controllerState.isLeftGripHeld)        ControllerFunctions::execute(ControllerCallbacks::leftGripOnClick);
        if(!oldControllerState.isButtonAHeld         && controllerState.isButtonAHeld)         ControllerFunctions::execute(ControllerCallbacks::buttonAOnClick);
        if(!oldControllerState.isButtonBHeld         && controllerState.isButtonBHeld)         ControllerFunctions::execute(ControllerCallbacks::buttonBOnClick);
        if(!oldControllerState.isRightThumbstickHeld && controllerState.isRightThumbstickHeld) ControllerFunctions::execute(ControllerCallbacks::rightThumbstickOnClick);
        if(!oldControllerState.isRightTriggerHeld    && controllerState.isRightTriggerHeld)    ControllerFunctions::execute(ControllerCallbacks::rightTriggerOnClick);
        if(!oldControllerState.isRightGripHeld       && controllerState.isRightGripHeld)       ControllerFunctions::execute(ControllerCallbacks::rightGripOnClick);

        ControllerFunctions::execute(ControllerCallbacks::buttonXOnHold,         controllerState.isButtonXHeld);
        ControllerFunctions::execute(ControllerCallbacks::buttonYOnHold,         controllerState.isButtonYHeld);
        ControllerFunctions::execute(ControllerCallbacks::leftThumbstickOnHold,  controllerState.isLeftThumbstickHeld);
        ControllerFunctions::execute(ControllerCallbacks::buttonAOnHold,         controllerState.isButtonAHeld);
        ControllerFunctions::execute(ControllerCallbacks::buttonBOnHold,         controllerState.isButtonBHeld);
        ControllerFunctions::execute(ControllerCallbacks::rightThumbstickOnHold, controllerState.isRightThumbstickHeld);

        ControllerFunctions::execute(ControllerCallbacks::leftTriggerOnHold,  controllerState.leftTriggerInput);
        ControllerFunctions::execute(ControllerCallbacks::leftGripOnHold,     controllerState.leftGripInput);
        ControllerFunctions::execute(ControllerCallbacks::rightTriggerOnHold, controllerState.rightTriggerInput);
        ControllerFunctions::execute(ControllerCallbacks::rightGripOnHold,    controllerState.rightGripInput);

        ControllerFunctions::execute(ControllerCallbacks::leftThumbstickHorizontalOnUpdate,  controllerState.leftThumbstickHorizontalInput);
        ControllerFunctions::execute(ControllerCallbacks::leftThumbstickVerticalOnUpdate,    controllerState.leftThumbstickVerticalInput);
        ControllerFunctions::execute(ControllerCallbacks::rightThumbstickHorizontalOnUpdate, controllerState.rightThumbstickHorizontalInput);
        ControllerFunctions::execute(ControllerCallbacks::rightThumbstickVerticalOnUpdate,   controllerState.rightThumbstickVerticalInput);
    }

}
