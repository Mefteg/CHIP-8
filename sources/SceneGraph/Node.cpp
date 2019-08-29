#include "SceneGraph/Node.hpp"

using namespace SceneGraph;

Node::Node()
{

}

void Node::setTransform(const sf::Transform& transform)
{
	m_transform = transform;
}

