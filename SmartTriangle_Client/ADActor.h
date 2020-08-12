#ifndef __ADACTOR_H__
#define __ADACTOR_H__

#include "ADConfig.h"
#include "ADAction.h"
#include "Vector.h"

class ADActor
{
private:
    ADAction *_actionVec_array[3];

    Vector<ADAction *> _actionVec;

    uint32_t _showTime = 0;

    uint32_t _showTime_c = 0;

    bool _autoDelete;

public:
    ADActor(uint32_t showTime , bool autoDelete = false);

    ~ADActor();

    static ADActor *create(uint32_t showTime , bool autoDelete = false);

    static ADActor *create(uint32_t showTime, ADAction *action , bool autoDelete = false);

    void release();

    void addAction(ADAction *action);

    void removeAction(ADAction *action, bool autoDelete = true);

    void removeAction(uint32_t index, bool autoDelete = true);

    bool show(uint32_t dt,bool autoSwitch);

    bool autoDelete(){return _autoDelete;}
};

#endif // __ADACTOR_H__
