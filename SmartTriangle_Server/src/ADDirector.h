#ifndef __ADDIRECTOR_H__
#define __ADDIRECTOR_H__

#include "ADConfig.h"
#include "ADActor.h"
#include "Vector.h"

class ADDirector
{
private:
    bool _invalid = false;

    bool _autoSwitch = false;

    uint16_t _autoSwitchPointer = 0;

    uint32_t _lastMillis = 0;

    ADActor *_actorVec_array[10];

    Vector<ADActor *> _actorVec;

public:
    ADDirector();

    ~ADDirector();

    void begin(bool autoSwitch = true);

    void loop(uint32_t ct);

    void startAction(uint32_t ct);

    void stopAction();

    void startAutoSwitch();

    void stopAutoSwitch();

    void addActor(ADActor *actor);

    void removeActor(ADActor *actor, bool autoDelete = true);

    void removeActor(uint32_t index, bool autoDelete = true);

    void flush();
};

extern ADDirector Director;

#endif // __ADDIRECTOR_H__