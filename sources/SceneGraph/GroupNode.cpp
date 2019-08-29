#include "SceneGraph/GroupNode.hpp"

using namespace SceneGraph;

GroupNode::GroupNode()
{

}

void GroupNode::addChild(Node* child)
{
	m_children.push_back(child);
}

void GroupNode::removeChild(std::size_t index)
{
	if (index >= m_children.size())
	{
		return;
	}

	m_children.erase(m_children.begin() + index);
}

void GroupNode::draw(sf::RenderTarget& target, const sf::Transform& parentTransform) const
{
    // Combine the parent transform with the node's one
	sf::Transform combinedTransform = parentTransform * m_transform;

    // Draw its children
	for (std::size_t i = 0; i < m_children.size(); ++i)
	{
		m_children[i]->draw(target, combinedTransform);
	}
}