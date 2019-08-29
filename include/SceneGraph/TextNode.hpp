#pragma once

#include "SceneGraph/Node.hpp"

namespace SceneGraph
{
	class TextNode : public Node
	{
	private:
		sf::Text m_text;

	public:
		TextNode();

		sf::Text& getText();

		virtual void draw(sf::RenderTarget& target, const sf::Transform& parentTransform) const;
	};	
}