#pragma once

#include <SFML/Graphics.hpp>

namespace SceneGraph
{
	class Node
	{
	protected:
		sf::Transform m_transform;

	public:
		Node();
		virtual ~Node() { };

		void setTransform(const sf::Transform& transform);

		virtual void draw(sf::RenderTarget& target, const sf::Transform& parentTransform) const = 0;
	};	
}
