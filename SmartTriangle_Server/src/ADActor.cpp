#include "ADActor.h"

ADActor::ADActor(uint32_t showTime, bool autoDelete)
{
    _showTime = showTime;
    _showTime_c = showTime;
    _autoDelete = autoDelete;
    _actionVec.setStorage(_actionVec_array);
}

ADActor::~ADActor()
{
}

ADActor *ADActor::create(uint32_t showTime, bool autoDelete)
{
    ADActor *actor = new ADActor(showTime, autoDelete);
    return actor;
}

ADActor *ADActor::create(uint32_t showTime, ADAction *action, bool autoDelete)
{
    ADActor *actor = new ADActor(showTime, autoDelete);
    actor->addAction(action);
    return actor;
}

void ADActor::release()
{
    for (int i = _actionVec.size() - 1; i >= 0; i--)
    {
        this->removeAction(i, true);
    }
    delete this;
}

void ADActor::addAction(ADAction *action)
{
    if (action)
    {
        _actionVec.push_back(action);
    }
}

void ADActor::removeAction(ADAction *action, bool autoDelete)
{
    for (int i = _actionVec.size() - 1; i >= 0; i--)
    {
        if (action == _actionVec[i])
        {
            _actionVec.remove(i);
            if (autoDelete)
            {
                action->release();
            }
            break;
        }
    }
}

void ADActor::removeAction(uint32_t index, bool autoDelete)
{
    ADAction *action = _actionVec[index];
    if (action)
    {
        _actionVec.remove(index);
        if (autoDelete)
        {
            action->release();
        }
    }
}

bool ADActor::show(uint32_t dt, bool autoSwitch)
{
    for (int i = _actionVec.size() - 1; i >= 0; i--)
    {
        ADAction *action = _actionVec[i];
        if (action->actNow(dt))
        {
            this->removeAction(i, autoSwitch);
        }
    }
    if (autoSwitch)
    {
        if (_showTime_c > dt)
        {
            _showTime_c -= dt;
        }
        else
        {
            _showTime_c = _showTime; // 产生些许时间误差
            return true;
        }
    }
    return false;
}
