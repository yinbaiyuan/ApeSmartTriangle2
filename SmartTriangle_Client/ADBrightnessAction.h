#ifndef __AD_BRIGHTNESS_ACTION_H__
#define __AD_BRIGHTNESS_ACTION_H__

#include "ADAction.h"

typedef void (*BrightnessActionCallback)(uint8_t, void *action);

class ADBrightnessAction : public ADAction
{
private:
    uint8_t _from;

    uint8_t _to;

    BrightnessActionCallback _callback;

    void callback();

public:
    ADBrightnessAction(BrightnessActionCallback func,
                uint8_t from,
                uint8_t to,
                uint32_t duration,
                uint32_t fps = 30,
                bool autoDelete = true);

    virtual ~ADBrightnessAction();

    static ADBrightnessAction *create(BrightnessActionCallback func,
                               uint8_t from,
                               uint8_t to,
                               uint32_t duration,
                               uint32_t fps = 30,
                               bool autoDelete = true);
};

#endif // __AD_BRIGHTNESS_ACTION_H__
