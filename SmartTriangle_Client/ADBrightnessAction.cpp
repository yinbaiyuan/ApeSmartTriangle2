#include "ADBrightnessAction.h"

ADBrightnessAction::ADBrightnessAction(BrightnessActionCallback func,
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

ADBrightnessAction::~ADBrightnessAction()
{
}

ADBrightnessAction *ADBrightnessAction::create(BrightnessActionCallback func,
    uint8_t from,
    uint8_t to,
    uint32_t duration,
    uint32_t fps,
    bool autoDelete)
{
  ADBrightnessAction *action = new ADBrightnessAction(func, from, to, duration, fps, autoDelete);
  return action;
}

void ADBrightnessAction::callback()
{
  uint8_t res = _to;
  if (_times != 0)
  {
    int32_t from = _from;
    int32_t to = _to;
    res = (from + (to - from) * (int32_t)_times_c / (int32_t)_times) % 256;
  }
  _callback(res, this);
}
