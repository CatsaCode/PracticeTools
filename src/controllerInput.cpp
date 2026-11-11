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
            using namespace MetaCore::Input;

            isButtonXHeld         = GetPressed(Controllers::Left,  Buttons::AX);
            isButtonYHeld         = GetPressed(Controllers::Left,  Buttons::BY);
            isLeftThumbstickHeld  = GetPressed(Controllers::Left,  Buttons::Thumbstick);
            isLeftTriggerHeld     = GetPressed(Controllers::Left,  Buttons::Trigger);
            isLeftGripHeld        = GetPressed(Controllers::Left,  Buttons::Grip);
            isButtonAHeld         = GetPressed(Controllers::Right, Buttons::AX);
            isButtonBHeld         = GetPressed(Controllers::Right, Buttons::BY);
            isRightThumbstickHeld = GetPressed(Controllers::Right, Buttons::Thumbstick);
            isRightTriggerHeld    = GetPressed(Controllers::Right, Buttons::Trigger);
            isRightGripHeld       = GetPressed(Controllers::Right, Buttons::Grip);

            leftThumbstickHorizontalInput  = GetAxis(Controllers::Left,  Axes::ThumbstickHorizontal);
            leftThumbstickVerticalInput    = GetAxis(Controllers::Left,  Axes::ThumbstickVertical);
            leftTriggerInput               = GetAxis(Controllers::Left,  Axes::TriggerStrength);
            leftGripInput                  = GetAxis(Controllers::Left,  Axes::GripStrength);
            rightThumbstickHorizontalInput = GetAxis(Controllers::Right, Axes::ThumbstickHorizontal);
            rightThumbstickVerticalInput   = GetAxis(Controllers::Right, Axes::ThumbstickVertical);
            rightTriggerInput              = GetAxis(Controllers::Right, Axes::TriggerStrength);
            rightGripInput                 = GetAxis(Controllers::Right, Axes::GripStrength);
        }
    };

    static ControllerState controllerState;
    void handleControllerInputs() {
        ControllerState oldControllerState = controllerState;
        controllerState.update();

        #define EXECUTE_CONTROLLER_FUNCTION(type, configName, ...) \
        ControllerFunctions::execute(static_cast<ControllerFunctions::type>(getPracticeToolsConfig().configName.GetValue()) __VA_OPT__(,) __VA_ARGS__)

        if(!oldControllerState.isButtonXHeld         && controllerState.isButtonXHeld)         EXECUTE_CONTROLLER_FUNCTION(ClickFunction, buttonXOnClick);
        if(!oldControllerState.isButtonYHeld         && controllerState.isButtonYHeld)         EXECUTE_CONTROLLER_FUNCTION(ClickFunction, buttonYOnClick);
        if(!oldControllerState.isLeftThumbstickHeld  && controllerState.isLeftThumbstickHeld)  EXECUTE_CONTROLLER_FUNCTION(ClickFunction, leftThumbstickOnClick);
        if(!oldControllerState.isLeftTriggerHeld     && controllerState.isLeftTriggerHeld)     EXECUTE_CONTROLLER_FUNCTION(ClickFunction, leftTriggerOnClick);
        if(!oldControllerState.isLeftGripHeld        && controllerState.isLeftGripHeld)        EXECUTE_CONTROLLER_FUNCTION(ClickFunction, leftGripOnClick);
        if(!oldControllerState.isButtonAHeld         && controllerState.isButtonAHeld)         EXECUTE_CONTROLLER_FUNCTION(ClickFunction, buttonAOnClick);
        if(!oldControllerState.isButtonBHeld         && controllerState.isButtonBHeld)         EXECUTE_CONTROLLER_FUNCTION(ClickFunction, buttonBOnClick);
        if(!oldControllerState.isRightThumbstickHeld && controllerState.isRightThumbstickHeld) EXECUTE_CONTROLLER_FUNCTION(ClickFunction, rightThumbstickOnClick);
        if(!oldControllerState.isRightTriggerHeld    && controllerState.isRightTriggerHeld)    EXECUTE_CONTROLLER_FUNCTION(ClickFunction, rightTriggerOnClick);
        if(!oldControllerState.isRightGripHeld       && controllerState.isRightGripHeld)       EXECUTE_CONTROLLER_FUNCTION(ClickFunction, rightGripOnClick);

        EXECUTE_CONTROLLER_FUNCTION(HoldFunction, buttonXOnHold, controllerState.isButtonXHeld);
        EXECUTE_CONTROLLER_FUNCTION(HoldFunction, buttonYOnHold, controllerState.isButtonYHeld);
        EXECUTE_CONTROLLER_FUNCTION(HoldFunction, leftThumbstickOnHold, controllerState.isLeftThumbstickHeld);
        EXECUTE_CONTROLLER_FUNCTION(HoldFunction, buttonAOnHold, controllerState.isButtonAHeld);
        EXECUTE_CONTROLLER_FUNCTION(HoldFunction, buttonBOnHold, controllerState.isButtonBHeld);
        EXECUTE_CONTROLLER_FUNCTION(HoldFunction, rightThumbstickOnHold, controllerState.isRightThumbstickHeld);

        EXECUTE_CONTROLLER_FUNCTION(HoldFunction, leftTriggerOnHold, controllerState.leftTriggerInput);
        EXECUTE_CONTROLLER_FUNCTION(HoldFunction, leftGripOnHold, controllerState.leftGripInput);
        EXECUTE_CONTROLLER_FUNCTION(HoldFunction, rightTriggerOnHold, controllerState.rightTriggerInput);
        EXECUTE_CONTROLLER_FUNCTION(HoldFunction, rightGripOnHold, controllerState.rightGripInput);

        EXECUTE_CONTROLLER_FUNCTION(ThumbstickFunction, leftThumbstickHorizontalOnUpdate, controllerState.leftThumbstickHorizontalInput);
        EXECUTE_CONTROLLER_FUNCTION(ThumbstickFunction, leftThumbstickVerticalOnUpdate, controllerState.leftThumbstickVerticalInput);
        EXECUTE_CONTROLLER_FUNCTION(ThumbstickFunction, rightThumbstickHorizontalOnUpdate, controllerState.rightThumbstickHorizontalInput);
        EXECUTE_CONTROLLER_FUNCTION(ThumbstickFunction, rightThumbstickVerticalOnUpdate, controllerState.rightThumbstickVerticalInput);
    }

}
