#include "ADDirector.h"

ADDirector::ADDirector(/* args */)
{
  _actorVec.setStorage(_actorVec_array);
}

ADDirector::~ADDirector()
{
}

void ADDirector::startAction(uint32_t ct)
{
  _lastMillis = ct;
  _invalid = false;
}

void ADDirector::stopAction()
{
  _invalid = true;
}

void ADDirector::startAutoSwitch()
{
  _autoSwitch = true;
}

void ADDirector::stopAutoSwitch()
{
  _autoSwitch = false;
}

void ADDirector ::begin(bool autoSwitch)
{
  _invalid = true;
  _autoSwitch = autoSwitch;
}
void ADDirector ::loop(uint32_t ct)
{
  if (_lastMillis >= ct)
    return;

  uint32_t dt = ct - _lastMillis;

  _lastMillis = ct;

  if (_invalid)
    return;
//  Serial.println("L0 " + String(_autoSwitchPointer) + " " + String(_actorVec.size()));

  for (int i = _actorVec.size() - 1; i >= 0; i--)
  {
    ADActor *actor = _actorVec[i];
    if (actor && actor->show(dt, _autoSwitch))
    {
      if (actor->autoDelete())
      {
        this->removeActor(actor);
      }
    }
  }

  //  ADActor *actor = _actorVec[_autoSwitchPointer];
  //
  //  if (actor && actor->show(dt, _autoSwitch))
  //  {
  //    if (actor->autoDelete())
  //    {
  //      this->removeActor(actor);
  //    }
  //    _autoSwitchPointer++;
  //    if (_autoSwitchPointer >= _actorVec.size())
  //    {
  //      _autoSwitchPointer = 0;
  //    }
  //  }

}

void ADDirector::addActor(ADActor *actor)
{
  _actorVec.push_back(actor);
}

void ADDirector::removeActor(ADActor *actor, bool autoDelete)
{
  for (int i = 0; i < _actorVec.size(); i++)
  {
    if (actor == _actorVec[i])
    {
      _actorVec.remove(i);
      if (autoDelete)
      {
        actor->release();
      }
      break;
    }
  }
}

void ADDirector::removeActor(uint32_t index, bool autoDelete)
{
  ADActor *actor = _actorVec[index];
  if (actor)
  {
    _actorVec.remove(index);
    if (autoDelete)
    {
      actor->release();
    }
  }
}

void ADDirector::flush()
{
  this->stopAction();
  for (int i = _actorVec.size() - 1; i >= 0; i--)
  {
    this->removeActor(i, true);
  }
}

ADDirector Director;
