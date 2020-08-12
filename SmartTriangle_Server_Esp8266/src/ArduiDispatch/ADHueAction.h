#ifndef __AD_HUE_ACTION_H__
#define __AD_HUE_ACTION_H__

#include "ADAction.h"

typedef void (*HueActionCallback)(uint32_t, void *action);

class ADHueAction : public ADAction
{
private:
    uint8_t _from;

    uint8_t _to;

    HueActionCallback _callback;

    void callback();

public:
    ADHueAction(HueActionCallback func,
                uint8_t from,
                uint8_t to,
                uint32_t duration,
                uint32_t fpsfps = 30,
                bool autoDelete = true);

    virtual ~ADHueAction();

    static ADHueAction *create(HueActionCallback func,
                               uint8_t from,
                               uint8_t to,
                               uint32_t duration,
                               uint32_t fpsfps = 30,
                               bool autoDelete = true);
};

#endif // __AD_HUE_ACTION_H__