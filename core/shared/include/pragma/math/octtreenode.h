/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __OCTTREENODE_H__
#define __OCTTREENODE_H__

#include "pragma/networkdefinitions.h"
#include <mathutil/glmutil.h>

template<int nodeCount>
class OctTreeNode;
class DLLNETWORK BaseOctTreeNode {
  public:
	template<int>
	friend class OctTreeNode;
  protected:
	BaseOctTreeNode(BaseOctTreeNode *parent = nullptr);
	BaseOctTreeNode *m_parent;
	unsigned char m_activeNodes;
  public:
	virtual ~BaseOctTreeNode() = default;
	virtual BaseOctTreeNode *GetParent();
	bool HasChildren();
	bool IsRoot();
	virtual unsigned char GetChildCount() = 0;
	virtual BaseOctTreeNode *GetChild(unsigned char idx) = 0;
};

template<int nodeCount>
class OctTreeNode : public BaseOctTreeNode {
  protected:
	OctTreeNode *m_children[nodeCount];
  public:
	OctTreeNode(OctTreeNode<nodeCount> *parent = nullptr);
	virtual ~OctTreeNode() override;
	virtual bool IsLeaf() = 0;
	unsigned char GetChildCount();
	OctTreeNode *GetParent();
	virtual OctTreeNode *GetChild(unsigned char idx) override;
};

template<int nodeCount>
OctTreeNode<nodeCount>::OctTreeNode(OctTreeNode<nodeCount> *parent) : BaseOctTreeNode(parent)
{
	for(unsigned int i = 0; i < nodeCount; i++)
		m_children[i] = nullptr;
}

template<int nodeCount>
OctTreeNode<nodeCount>::~OctTreeNode()
{
	for(unsigned int i = 0; i < nodeCount; i++) {
		if(m_children[i] != nullptr)
			delete m_children[i];
	}
}

template<int nodeCount>
unsigned char OctTreeNode<nodeCount>::GetChildCount()
{
	return nodeCount;
}
template<int nodeCount>
OctTreeNode<nodeCount> *OctTreeNode<nodeCount>::GetParent()
{
	return static_cast<OctTreeNode<nodeCount> *>(m_parent);
}
template<int nodeCount>
OctTreeNode<nodeCount> *OctTreeNode<nodeCount>::GetChild(unsigned char idx)
{
	return m_children[idx];
}

#endif
