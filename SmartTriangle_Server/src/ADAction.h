#ifndef __ADACTION_H__
#define __ADACTION_H__

#include "ADConfig.h"

typedef void (*ActionCallback)(uint32_t, void *action);

class ADAction
{
private:
    ActionCallback _callback;

    uint32_t _delay;

    uint32_t _interval;

    uint32_t _delay_c;

    uint32_t _interval_c;

    bool _autoDelete;

    void callbackAction();

    virtual void callback();

protected:
    uint32_t _times;

    uint32_t _times_c;

public:
    ADAction(ActionCallback func, uint32_t interval, uint32_t times = 0, uint32_t delay = 0, bool autoDelete = true);

    virtual ~ADAction();

    //延迟delay毫秒，每间隔interval毫秒执行一次func,共执行times次。如果times==0，一直重复执行
    static ADAction *create(ActionCallback func, uint32_t interval, uint32_t times = 0, uint32_t delay = 0, bool autoDelete = true);

    void release();

    void flush();

    virtual bool actNow(uint32_t dt);
};

#endif // __ADACTION_H__