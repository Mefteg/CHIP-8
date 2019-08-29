#pragma once

#include "SceneGraph/Node.hpp"

#include <vector>

namespace SceneGraph
{
	class GroupNode : public Node
	{
	private:
		std::vector<Node*> m_children;

	public:
		GroupNode();

		void addChild(Node* child);
		void removeChild(std::size_t index);

		virtual void draw(sf::RenderTarget& target, const sf::Transform& parentTransform) const;
	};	
}