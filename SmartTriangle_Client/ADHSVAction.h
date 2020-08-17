#ifndef __AD_HSV_ACTION_H__
#define __AD_HSV_ACTION_H__

#include "ADAction.h"

typedef void (*HSVActionCallback)(uint8_t,uint8_t,uint8_t,void *action);

class ADHSVAction : public ADAction
{
private:
    uint8_t _from[3];

    uint8_t _to[3];

    HSVActionCallback _callback;

    void callback();

public:
    ADHSVAction(HSVActionCallback func,
                uint8_t *from,
                uint8_t *to,
                uint32_t duration,
                uint32_t fps = 30,
                bool autoDelete = true);

    virtual ~ADHSVAction();

    static ADHSVAction *create(HSVActionCallback func,
                               uint8_t *from,
                               uint8_t *to,
                               uint32_t duration,
                               uint32_t fps = 30,
                               bool autoDelete = true);
};

#endif // __AD_HSV_ACTION_H__
