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

        #define JUST_CLICKED(buttonName) !oldControllerState.is##buttonName##Held && controllerState.is##buttonName##Held
        #define ANALOG_ACTIVE(analogName) abs(controllerState.analogName##Input) > getPracticeToolsConfig().deadband.GetValue()
        #define EXECUTE_CONTROLLER_FUNCTION(type, configName, ...) \
        ControllerFunctions::execute(static_cast<ControllerFunctions::type>(getPracticeToolsConfig().configName.GetValue()) __VA_OPT__(,) __VA_ARGS__)

        if (JUST_CLICKED(ButtonX))         EXECUTE_CONTROLLER_FUNCTION(ClickFunction, buttonXOnClick);
        if (JUST_CLICKED(ButtonY))         EXECUTE_CONTROLLER_FUNCTION(ClickFunction, buttonYOnClick);
        if (JUST_CLICKED(LeftTrigger))     EXECUTE_CONTROLLER_FUNCTION(ClickFunction, leftTriggerOnClick);
        if (JUST_CLICKED(LeftGrip))        EXECUTE_CONTROLLER_FUNCTION(ClickFunction, leftGripOnClick);
        if (JUST_CLICKED(LeftThumbstick))  EXECUTE_CONTROLLER_FUNCTION(ClickFunction, leftThumbstickOnClick);
        if (JUST_CLICKED(ButtonA))         EXECUTE_CONTROLLER_FUNCTION(ClickFunction, buttonAOnClick);
        if (JUST_CLICKED(ButtonB))         EXECUTE_CONTROLLER_FUNCTION(ClickFunction, buttonBOnClick);
        if (JUST_CLICKED(RightTrigger))    EXECUTE_CONTROLLER_FUNCTION(ClickFunction, rightTriggerOnClick);
        if (JUST_CLICKED(RightGrip))       EXECUTE_CONTROLLER_FUNCTION(ClickFunction, rightGripOnClick);
        if (JUST_CLICKED(RightThumbstick)) EXECUTE_CONTROLLER_FUNCTION(ClickFunction, rightThumbstickOnClick);

        if (controllerState.isButtonXHeld)         EXECUTE_CONTROLLER_FUNCTION(HoldFunction, buttonXOnHold, 1);
        if (controllerState.isButtonYHeld)         EXECUTE_CONTROLLER_FUNCTION(HoldFunction, buttonYOnHold, 1);
        if (controllerState.isLeftThumbstickHeld)  EXECUTE_CONTROLLER_FUNCTION(HoldFunction, leftThumbstickOnHold, 1);
        if (controllerState.isButtonAHeld)         EXECUTE_CONTROLLER_FUNCTION(HoldFunction, buttonAOnHold, 1);
        if (controllerState.isButtonBHeld)         EXECUTE_CONTROLLER_FUNCTION(HoldFunction, buttonBOnHold, 1);
        if (controllerState.isRightThumbstickHeld) EXECUTE_CONTROLLER_FUNCTION(HoldFunction, rightThumbstickOnHold, 1);

        if (ANALOG_ACTIVE(leftTrigger))  EXECUTE_CONTROLLER_FUNCTION(HoldFunction, leftTriggerOnHold, controllerState.leftTriggerInput);
        if (ANALOG_ACTIVE(leftGrip))     EXECUTE_CONTROLLER_FUNCTION(HoldFunction, leftGripOnHold, controllerState.leftGripInput);
        if (ANALOG_ACTIVE(rightTrigger)) EXECUTE_CONTROLLER_FUNCTION(HoldFunction, rightTriggerOnHold, controllerState.rightTriggerInput);
        if (ANALOG_ACTIVE(rightGrip))    EXECUTE_CONTROLLER_FUNCTION(HoldFunction, rightGripOnHold, controllerState.rightGripInput);

        if (ANALOG_ACTIVE(leftThumbstickHorizontal))  EXECUTE_CONTROLLER_FUNCTION(ThumbstickFunction, leftThumbstickHorizontalOnUpdate, controllerState.leftThumbstickHorizontalInput);
        if (ANALOG_ACTIVE(leftThumbstickVertical))    EXECUTE_CONTROLLER_FUNCTION(ThumbstickFunction, leftThumbstickVerticalOnUpdate, controllerState.leftThumbstickVerticalInput);
        if (ANALOG_ACTIVE(rightThumbstickHorizontal)) EXECUTE_CONTROLLER_FUNCTION(ThumbstickFunction, rightThumbstickHorizontalOnUpdate, controllerState.rightThumbstickHorizontalInput);
        if (ANALOG_ACTIVE(rightThumbstickVertical))   EXECUTE_CONTROLLER_FUNCTION(ThumbstickFunction, rightThumbstickVerticalOnUpdate, controllerState.rightThumbstickVerticalInput);
    }

}
