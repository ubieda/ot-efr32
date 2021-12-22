/* Header for the functions defined here */
#include "openthread-system.h"

#include <string.h>

/* Header to access an OpenThread instance */
#include <openthread/instance.h>

/* Headers for lower-level efr32 functions */
#include <platform/emlib/inc/em_gpio.h>
#include <gpiointerrupt.h>
#include "hal-config.h"

struct gpioDrv {
    uint32_t port;
    uint32_t pin;
};

struct gpioDrv leds[] = {
    {.port = gpioPortD, .pin = 2,},
    {.port = 0xFF, .pin = 0xFF,}, /* Dummy gpio */
    {.port = 0xFF, .pin = 0xFF,}, /* Dummy gpio */
    {.port = gpioPortD, .pin = 3,},
};

struct gpioDrv buttons[] = {
    {.port = gpioPortB, .pin = 0,},
    {.port = gpioPortB, .pin = 1,},
};

void otSysLedInit(void)
{

    for (size_t i = 0 ; i < sizeof(leds)/sizeof(leds[0]) ; i++) {
        GPIO_PinModeSet(leds[i].port, leds[i].pin, gpioModePushPull, 1);
    }
}

void otSysLedSet(uint8_t aLed, bool aOn)
{
    if (aLed > sizeof(leds)/sizeof(leds[0])) {
        return;
    }

    if (aOn) {
        GPIO_PinOutSet(leds[aLed - 1].port, leds[aLed - 1].pin);
    } else {
        GPIO_PinOutClear(leds[aLed - 1].port, leds[aLed - 1].pin);
    }
}

void otSysLedToggle(uint8_t aLed)
{
    if (aLed > sizeof(leds)/sizeof(leds[0])) {
        return;
    }

    GPIO_PinOutToggle(leds[aLed - 1].port, leds[aLed - 1].pin);
}

static uint8_t button_pressed = 0;
static otSysButtonCallback user_handler;

static void button_handler(uint8_t pin)
{
    for (size_t i = 0 ; i < sizeof(buttons)/sizeof(buttons[0]) ; i++) {
        if (pin == buttons[i].pin) {
            button_pressed = i + 1;
        }
    }
}

void otSysButtonInit(otSysButtonCallback aCallback)
{
    if (aCallback) {
        user_handler = aCallback;
    }

    GPIOINT_Init();

    for (size_t i = 0 ; i < sizeof(buttons)/sizeof(buttons[0]) ; i++) {
        GPIO_PinModeSet(buttons[i].port, buttons[i].pin, gpioModeInput, 0);
        GPIOINT_CallbackRegister(buttons[i].pin, button_handler);

        GPIO_ExtIntConfig(buttons[i].port,
                        buttons[i].pin,
                        buttons[i].pin,
                        false,
                        true,
                        true);
    }
}

void otSysButtonProcess(otInstance *aInstance)
{
    if (button_pressed) {
        if (user_handler) {
            user_handler(aInstance, button_pressed);
        }
        button_pressed = 0;
    }
}