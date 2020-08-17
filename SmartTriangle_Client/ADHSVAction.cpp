#include "ADHSVAction.h"

ADHSVAction::ADHSVAction(HSVActionCallback func,
                         uint8_t *from,
                         uint8_t *to,
                         uint32_t duration,
                         uint32_t fps,
                         bool autoDelete) : ADAction(NULL, 1000 / fps, duration * fps / 1000.0, 0, autoDelete)
{
  _callback = func;
  for(int i = 0;i<3;i++)
  {
    _from[i] = from[i];
    _to[i] = to[i];
  }
}

ADHSVAction::~ADHSVAction()
{
}

ADHSVAction *ADHSVAction::create(HSVActionCallback func,
                                 uint8_t *from,
                                 uint8_t *to,
                                 uint32_t duration,
                                 uint32_t fps,
                                 bool autoDelete)
{
  ADHSVAction *action = new ADHSVAction(func, from, to, duration, fps, autoDelete);
  return action;
}

void ADHSVAction::callback()
{
  uint8_t res[3] = {_to[0],_to[1],_to[2]};
  if (_times != 0)
  {
    int32_t from[3] = {_from[0],_from[1],_from[2]};
    int32_t to[3] = {_to[0],_to[1],_to[2]};
    if (from[0] > to[0])
    {
      to[0] += 256;
    }
    for(int i = 0;i<3;i++)
    {
      res[i] = (from[i] + (to[i] - from[i]) * (int32_t)_times_c / (int32_t)_times) % 256;
    }
  }
  _callback(res[0],res[1],res[2],this);
}
