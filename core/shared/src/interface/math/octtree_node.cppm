// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:math.octtree_node;

export {
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
		OctTreeNode(OctTreeNode<nodeCount> *parent = nullptr) : BaseOctTreeNode(parent)
		{
			for(unsigned int i = 0; i < nodeCount; i++)
				m_children[i] = nullptr;
		}
		virtual ~OctTreeNode() override
		{
			for(unsigned int i = 0; i < nodeCount; i++) {
				if(m_children[i] != nullptr)
					delete m_children[i];
			}
		}
		virtual bool IsLeaf() = 0;
		unsigned char GetChildCount() { return nodeCount; }
		OctTreeNode *GetParent() { return static_cast<OctTreeNode<nodeCount> *>(m_parent); }
		virtual OctTreeNode *GetChild(unsigned char idx) override { return m_children[idx]; }
	};
};
