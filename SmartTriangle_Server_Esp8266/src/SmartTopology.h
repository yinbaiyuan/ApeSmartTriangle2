#ifndef SmartTopology_h
#define SmartTopology_h

#include <inttypes.h>
#include <Arduino.h>

enum STNodeType
{
    STNT_WAITING_CHECK = 0,
    STNT_CHECKING = 1,
    STNT_HAS_NO_CHILD,
    STNT_HAS_CHILD
};

struct STNodeDef
{
  uint8_t nodeId;
  STNodeType leftChildType;
  STNodeType rightChildType;
  STNodeDef *leftChild;
  STNodeDef *rightChild;
};

class SmartTopology
{
  private:
    STNodeDef *m_rootNode;
    uint8_t creadId();
    void deleteTree(STNodeDef* node);
  public:
    SmartTopology();
    ~SmartTopology();
    STNodeDef *creatNode();
    STNodeDef *rootNode();
    // void nextNode(STNodeDef* node);
    uint8_t creatRootNode();
    uint8_t nodeCount();
    void flush();
};

extern SmartTopology ST;

#endif
