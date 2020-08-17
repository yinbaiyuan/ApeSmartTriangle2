#include "SmartTopology.h"
#include <Arduino.h>
#include <Vector.h>

STNodeDef *stNodeStack_storage[64];
Vector<STNodeDef *> stNodeStack;

static uint8_t idCreator = 1;

SmartTopology::SmartTopology()
{
  idCreator = 1;
  m_rootNode = NULL;
  stNodeStack.setStorage(stNodeStack_storage);
}

SmartTopology::~SmartTopology()
{
  if (m_rootNode)
  {
    delete m_rootNode;
    m_rootNode = NULL;
  }
}

uint8_t SmartTopology::creadId()
{
  return idCreator++;
}

STNodeDef *SmartTopology::creatNode()
{
  STNodeDef *node = new STNodeDef();
  memset(node, 0, sizeof(STNodeDef));
  node->nodeId = this->creadId();
  stNodeStack.push_back(node);
  return node;
}

uint8_t SmartTopology::creatRootNode()
{
  m_rootNode = this->creatNode();
  return m_rootNode->nodeId;
}

uint8_t SmartTopology::nodeCount()
{
  return stNodeStack.size();
}

STNodeDef *SmartTopology::rootNode()
{
  return m_rootNode;
}

// //深度优先遍历
// void SmartTopology::nextNode(STNodeDef* node)
// {
//   int stackSize = stNodeStack.size();
//   if(stackSize<=0)
//   {
//     m_currentNode = m_rootNode;
//     stNodeStack.push_back(m_currentNode);
//     return;
//   }
//   STNodeDef* topNode = stNodeStack.at(stackSize - 1);
//   if (topNode == node)
//   {
//     if (node->leftChild)
//     {
//       m_currentNode = node->leftChild;
//       stNodeStack.push_back(m_currentNode);
//     } else if (node->rightChild)
//     {
//       m_currentNode = node->rightChild;
//       stNodeStack.push_back(m_currentNode);
//     } else
//     {
//       stNodeStack.pop_back();
//       this->nextNode(node);
//     }
//   } else if (topNode->leftChild == node)
//   {
//     if (topNode->rightChild)
//     {
//       m_currentNode = topNode->rightChild;
//       stNodeStack.push_back(m_currentNode);
//     }else
//     {
//       stNodeStack.pop_back();
//       this->nextNode(topNode);
//     }
//   }else if (topNode->rightChild == node)
//   {
//       stNodeStack.pop_back();
//       this->nextNode(node);
//   }
// }

void SmartTopology::flush()
{
  idCreator = 1;
  this->deleteTree(m_rootNode);
}

void SmartTopology::deleteTree(STNodeDef *node)
{
  if (node)
  {
    this->deleteTree(node->leftChild);
    this->deleteTree(node->rightChild);
    delete node;
  }
}
uint8_t *randomCache = NULL;
uint8_t randomLength = 0;

void SmartTopology::fullRandomInit(uint8_t n)
{
  if (randomCache)
  {
    delete randomCache;
    randomCache = NULL;
  }
  if (n > 0)
  {
    randomCache = (uint8_t *)malloc(n);
    randomLength = n;
    for(int i = 0;i<n;i++)
    {
      randomCache[i] = i;
    }
  }
}

uint8_t SmartTopology::fullRandom()
{
  if(randomLength<=0)
  {
    return 0;
  }
  int pointer = random(0,randomLength);
  uint8_t res = randomCache[pointer];
  randomCache[pointer] = randomCache[--randomLength];
  return res;
}
SmartTopology ST;
