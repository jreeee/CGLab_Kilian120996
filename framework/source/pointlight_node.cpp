#include "pointlight_node.hpp"

PointLightNode::PointLightNode():
    Node(), 
    intensity_ {1.0f} {

}

PointLightNode::PointLightNode( std::shared_ptr<Node> parent,
                                std::string const& name,
                                float intensity):
    Node(parent, name),
    intensity_ {intensity} {

}

PointLightNode::~PointLightNode() {}

float PointLightNode::getIntensity() const {
    return intensity_;
}

void PointLightNode::setIntensity(float intensity) {
    intensity_ = abs(intensity);
}

void PointLightNode::printChildrenList(std::stringstream & output) {
    output << "\n( P | " << depth_ << " ) " << name_;
    for (auto const& i : children_) {
        i->printChildrenList(output);
    }
}