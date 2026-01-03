// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :rendering.occlusion_culling.chc_new;

#if 0
static const uint32_t maxPrevInvisNodesBatchSize = 50; // Recommended range: [20,80]

void pragma::CHCNode::SetVisible(bool bVisible) {m_bVisible = bVisible;}
bool pragma::CHCNode::IsVisible() const {return m_bVisible;}

////////////

std::vector<CullingMeshInfo> &pragma::CHC::PerformCulling()
{
	m_distanceQueue.push(m_rootNode);
	while(m_distanceQueue.empty() == false || m_queryQueue.empty() == false)
	{
		while(m_queryQueue.empty() == false)
		{
			if(firstQueryFinished())
			{
				auto query = m_queryQueue.front();
				m_queryQueue.pop();
				HandleReturnedQuery(*query);
			}
			else
			{
				auto visNode = m_visQueue.front();
				m_visQueue.pop();
				if(visNode.valid())
					IssueQuery(*visNode);
			}
		}
		if(m_distanceQueue.empty() == false)
		{
			auto node = m_distanceQueue.front();
			m_distanceQueue.pop();
			if(node.valid() && InsideViewFrustum(*node))
			{
				if(WasVisible(*node) == false)
					QueryPreviouslyInvisibleNode(*node);
				else
				{
					if(node->IsLeaf() && QueryReasonable(*node))
						m_visQueue.push(node);
					TraverseNode(*node);
				}
			}
		}
		if(m_distanceQueue.empty() == true)
			IssueMultiQueries();
	}
	while(m_visQueue.empty() == false)
	{
		auto node = m_visQueue.front();
		m_visQueue.pop();
		if(node.valid())
			IssueQuery(*node);
	}
}
void pragma::CHC::TraverseNode(CHCNode &node)
{
	if(node.IsLeaf())
	{
		Render(node);
		return;
	}
	for(auto &pChild : node.GetChildren())
		m_distanceQueue.push(pChild);
	node.SetVisible(false);
}
void pragma::CHC::PullUpVisibility(CHCNode &node)
{
	auto *pNode = &node;
	while(pNode != nullptr && pNode->IsVisible() == false)
	{
		pNode->SetVisible(true);
		pNode = pNode->GetParent();
	}
}
void pragma::CHC::QueryPreviouslyInvisibleNode(CHCNode &cNode)
{
	m_invisQueue.push(cNode.shared_from_this());
	if(m_invisQueue.size() >= maxPrevInvisNodesBatchSize)
		IssueMultiQueries();
}
bool pragma::CHC::InsideViewFrustum(CHCNode &cNode)
{
	 // TODO
}
bool pragma::CHC::QueryReasonable(CHCNode &cNode)
{
	// TODO
}
void pragma::CHC::Render(CHCNode &cNode)
{
	// TODO
}
bool pragma::CHC::WasVisible(CHCNode &cNode)
{
	// TODO
}
void pragma::CHC::InitializeNodes(const BaseOcclusionOctree::Node &node)
{
	// TODO
}

void pragma::CHC::IssueMultiQueries()
{
	while(m_invisQueue.empty() == false)
	{
		mq = m_invisQueue.GetNextMultiQuery();
		IssueQuery(mq);
		m_invisQueue.PopNodes(mq);
	}
}
void pragma::CHC::HandleReturnedQuery(CHCQuery &query)
{
	auto *pNode = query.GetNode();
	if(query.GetVisiblePixels() > threshold)
	{
		if(query.GetQueryCount() > 1)
			QueryIndividualNodes(query);
		else if(pNode != nullptr)
		{
			if(WasVisible(*pNode) == false)
				TraverseNode(*pNode);
			PullUpVisibility(*pNode);
		}
	}
	else if(pNode != nullptr)
		pNode->SetVisible(false);
}
void pragma::CHC::IssueQuery(CHCNode &node)
{
	// TODO
}
void pragma::CHC::UpdateFrustum()
{
	// TODO
}
#endif
