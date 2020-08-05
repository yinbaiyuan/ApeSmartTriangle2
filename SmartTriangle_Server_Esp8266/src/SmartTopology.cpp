#include "SmartTopology.h"
#include <Arduino.h>
#include <Vector.h>

STNodeDef* stNodeStack_storage[64];
Vector<STNodeDef *> stNodeStack;

static uint8_t idCreator = 0;

SmartTopology::SmartTopology()
{
  idCreator = 0;
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
  return node;
}

uint8_t SmartTopology::creatRootNode()
{
  m_rootNode = this->creatNode();
  m_currentNode = m_rootNode;
  stNodeStack.push_back(m_currentNode);
  return m_rootNode->nodeId;
}


STNodeDef *SmartTopology::rootNode()
{
  return m_rootNode;
}

STNodeDef *SmartTopology::currentNode()
{
  return m_currentNode;
}

//深度优先遍历
void SmartTopology::nextNode(STNodeDef* node)
{
  int stackSize = stNodeStack.size();
  if(stackSize<=0)
  {
    m_currentNode = m_rootNode;
    stNodeStack.push_back(m_currentNode);
    return;
  }
  STNodeDef* topNode = stNodeStack.at(stackSize - 1);
  if (topNode == node)
  {
    if (node->leftLeaf)
    {
      m_currentNode = node->leftLeaf;
      stNodeStack.push_back(m_currentNode);
    } else if (node->rightLeaf)
    {
      m_currentNode = node->rightLeaf;
      stNodeStack.push_back(m_currentNode);
    } else
    {
      stNodeStack.pop_back();
      this->nextNode(node);
    }
  } else if (topNode->leftLeaf == node)
  {
    if (topNode->rightLeaf)
    {
      m_currentNode = topNode->rightLeaf;
      stNodeStack.push_back(m_currentNode);
    }else
    {
      stNodeStack.pop_back();
      this->nextNode(topNode);
    }
  }else if (topNode->rightLeaf == node)
  {
      stNodeStack.pop_back();
      this->nextNode(node);
  }
}

void SmartTopology::flush()
{
  idCreator = 0;
  this->deleteTree(m_rootNode);
}

void SmartTopology::deleteTree(STNodeDef* node)
{
  if (node)
  {
    this->deleteTree(node->leftLeaf);
    this->deleteTree(node->rightLeaf);
    delete node;
  }
}

SmartTopology ST;
