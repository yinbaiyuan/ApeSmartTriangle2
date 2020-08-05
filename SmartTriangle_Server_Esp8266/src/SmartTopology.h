#ifndef SmartTopology_h
#define SmartTopology_h

#include <inttypes.h>
#include <Arduino.h>

struct STNodeDef
{
  uint8_t nodeId;
  STNodeDef *leftLeaf;
  STNodeDef *rightLeaf;
};

class SmartTopology
{
  private:
    STNodeDef *m_rootNode;
    STNodeDef *m_currentNode;
    uint8_t creadId();
    void deleteTree(STNodeDef* node);
  public:
    SmartTopology();
    ~SmartTopology();
    STNodeDef *creatNode();
    STNodeDef *rootNode();
    STNodeDef *currentNode();
    void nextNode(STNodeDef* node);
    uint8_t creatRootNode();
    void flush();
};

extern SmartTopology ST;

#endif
