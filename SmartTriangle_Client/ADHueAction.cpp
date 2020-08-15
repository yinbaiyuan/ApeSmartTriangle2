#include "ADHueAction.h"

ADHueAction::ADHueAction(HueActionCallback func,
                         uint8_t from,
                         uint8_t to,
                         uint32_t duration,
                         uint32_t fps,
                         bool autoDelete) : ADAction(NULL, 1000 / fps, duration * fps / 1000.0, 0, autoDelete)
{
  _callback = func;
  _from = from;
  _to = to;
}

ADHueAction::~ADHueAction()
{
}

ADHueAction *ADHueAction::create(HueActionCallback func,
                                 uint8_t from,
                                 uint8_t to,
                                 uint32_t duration,
                                 uint32_t fps,
                                 bool autoDelete)
{
  ADHueAction *action = new ADHueAction(func, from, to, duration, fps, autoDelete);
  return action;
}

void ADHueAction::callback()
{
  uint8_t res = _to;
  if (_times != 0)
  {
    int32_t from = _from;
    int32_t to = _to;
    if (from > to)
    {
      to += 256;
    }
    res = ((to - from) * _times_c / _times + from) % 256;
  }
  _callback(res, this);
}
