#include "main.hpp"

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

        if(!oldControllerState.isButtonXHeld         && controllerState.isButtonXHeld)         ControllerFunctions::invoke(ControllerFunctions::clickFunctions, ControllerCallbacks::buttonXOnClick);
        if(!oldControllerState.isButtonYHeld         && controllerState.isButtonYHeld)         ControllerFunctions::invoke(ControllerFunctions::clickFunctions, ControllerCallbacks::buttonYOnClick);
        if(!oldControllerState.isLeftThumbstickHeld  && controllerState.isLeftThumbstickHeld)  ControllerFunctions::invoke(ControllerFunctions::clickFunctions, ControllerCallbacks::leftThumbstickOnClick);
        if(!oldControllerState.isLeftTriggerHeld     && controllerState.isLeftTriggerHeld)     ControllerFunctions::invoke(ControllerFunctions::clickFunctions, ControllerCallbacks::leftTriggerOnClick);
        if(!oldControllerState.isLeftGripHeld        && controllerState.isLeftGripHeld)        ControllerFunctions::invoke(ControllerFunctions::clickFunctions, ControllerCallbacks::leftGripOnClick);
        if(!oldControllerState.isButtonAHeld         && controllerState.isButtonAHeld)         ControllerFunctions::invoke(ControllerFunctions::clickFunctions, ControllerCallbacks::buttonAOnClick);
        if(!oldControllerState.isButtonBHeld         && controllerState.isButtonBHeld)         ControllerFunctions::invoke(ControllerFunctions::clickFunctions, ControllerCallbacks::buttonBOnClick);
        if(!oldControllerState.isRightThumbstickHeld && controllerState.isRightThumbstickHeld) ControllerFunctions::invoke(ControllerFunctions::clickFunctions, ControllerCallbacks::rightThumbstickOnClick);
        if(!oldControllerState.isRightTriggerHeld    && controllerState.isRightTriggerHeld)    ControllerFunctions::invoke(ControllerFunctions::clickFunctions, ControllerCallbacks::rightTriggerOnClick);
        if(!oldControllerState.isRightGripHeld       && controllerState.isRightGripHeld)       ControllerFunctions::invoke(ControllerFunctions::clickFunctions, ControllerCallbacks::rightGripOnClick);

        ControllerFunctions::invoke<float>(ControllerFunctions::holdFunctions, ControllerCallbacks::buttonXOnHold,         controllerState.isButtonXHeld);
        ControllerFunctions::invoke<float>(ControllerFunctions::holdFunctions, ControllerCallbacks::buttonYOnHold,         controllerState.isButtonYHeld);
        ControllerFunctions::invoke<float>(ControllerFunctions::holdFunctions, ControllerCallbacks::leftThumbstickOnHold,  controllerState.isLeftThumbstickHeld);
        ControllerFunctions::invoke<float>(ControllerFunctions::holdFunctions, ControllerCallbacks::buttonAOnHold,         controllerState.isButtonAHeld);
        ControllerFunctions::invoke<float>(ControllerFunctions::holdFunctions, ControllerCallbacks::buttonBOnHold,         controllerState.isButtonBHeld);
        ControllerFunctions::invoke<float>(ControllerFunctions::holdFunctions, ControllerCallbacks::rightThumbstickOnHold, controllerState.isRightThumbstickHeld);

        ControllerFunctions::invoke<float>(ControllerFunctions::holdFunctions, ControllerCallbacks::leftTriggerOnHold,   controllerState.leftTriggerInput);
        ControllerFunctions::invoke<float>(ControllerFunctions::holdFunctions, ControllerCallbacks::leftGripOnHold,      controllerState.leftGripInput);
        ControllerFunctions::invoke<float>(ControllerFunctions::holdFunctions, ControllerCallbacks::rightTriggerOnHold,  controllerState.rightTriggerInput);
        ControllerFunctions::invoke<float>(ControllerFunctions::holdFunctions, ControllerCallbacks::rightGripOnHold,     controllerState.rightGripInput);

        ControllerFunctions::invoke<float>(ControllerFunctions::thumbstickFunctions, ControllerCallbacks::leftThumbstickHorizontalOnUpdate,  controllerState.leftThumbstickHorizontalInput);
        ControllerFunctions::invoke<float>(ControllerFunctions::thumbstickFunctions, ControllerCallbacks::leftThumbstickVerticalOnUpdate,    controllerState.leftThumbstickVerticalInput);
        ControllerFunctions::invoke<float>(ControllerFunctions::thumbstickFunctions, ControllerCallbacks::rightThumbstickHorizontalOnUpdate, controllerState.rightThumbstickHorizontalInput);
        ControllerFunctions::invoke<float>(ControllerFunctions::thumbstickFunctions, ControllerCallbacks::rightThumbstickVerticalOnUpdate,   controllerState.rightThumbstickVerticalInput);
    }

}
