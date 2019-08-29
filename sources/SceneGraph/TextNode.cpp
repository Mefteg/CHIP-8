#include "SceneGraph/TextNode.hpp"

using namespace SceneGraph;

TextNode::TextNode()
{

}

sf::Text& TextNode::getText()
{
	return m_text;
}

void TextNode::draw(sf::RenderTarget& target, const sf::Transform& parentTransform) const
{
    // Combine the parent transform with the node's one
	sf::Transform combinedTransform = parentTransform * m_transform;

	target.draw(m_text, combinedTransform);
}