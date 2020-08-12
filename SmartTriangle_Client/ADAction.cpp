#include "ADAction.h"

ADAction::ADAction(ActionCallback func,uint32_t interval, uint32_t times,uint32_t delay,  bool autoDelete)
{
    _callback = func;
    _delay = delay;
    _delay_c = delay;
    _interval = interval;
    _interval_c = interval;
    _times = times;
    _times_c = 0;
    _autoDelete = autoDelete;
}

ADAction::~ADAction()
{
}

void ADAction::callbackAction()
{
    _times_c++;
    this->callback();
}

void ADAction::callback()
{
    ADLOG_S("ADAction callback");
    _callback(_times_c, this);
}

ADAction *ADAction::create(ActionCallback func, uint32_t interval, uint32_t times,uint32_t delay,  bool autoDelete)
{
    ADAction *action = new ADAction(func, delay, interval, times, autoDelete);
    return action;
}

void ADAction::release()
{
    delete this;
}

void ADAction::flush()
{
    _times_c = 0;
    _delay_c = _delay;
    _interval_c = _interval;
}

bool ADAction::actNow(uint32_t dt)
{
    if (_delay_c > dt)
    {
        _delay_c -= dt;
    }
    else
    {
        _delay_c = 0;
        if (_times_c == 0)
        {
            this->callbackAction();
        }
        if (_interval_c > dt)
        {
            _interval_c -= dt;
        }
        else
        {
            _interval_c = 0;
        }
    }
    if (_delay_c == 0 && _interval_c == 0)
    {
        if (_times != 0 && _times_c >= _times)
        {
            this->flush();
            if (_autoDelete)
            {
                return true;
            }
        }else
        {
            this->callbackAction();
            _interval_c = _interval;
        }
    }
    return false;
}
