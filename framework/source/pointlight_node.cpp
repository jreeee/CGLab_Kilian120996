#include "pointlight_node.hpp"

PointLightNode::PointLightNode():
    Node(), 
    lightIntensity_ {1.0f},
    lightColor_ {{0.5f},{0.5f},{0.5f}} {
}

PointLightNode::PointLightNode( std::shared_ptr<Node> parent,
                                std::string const& name,
                                Color const& lightColor,
                                float lightIntensity):
    Node(parent, name),
    lightColor_ {lightColor},
    lightIntensity_ {lightIntensity} {

}

PointLightNode::~PointLightNode() {}

float PointLightNode::getIntensity() const {
    return lightIntensity_;
}

void PointLightNode::setIntensity(float intensity) {
    lightIntensity_ = abs(intensity);
}

Color PointLightNode::getLightColor() const {
    return lightColor_;
}

void PointLightNode::setLightColor(Color const& color) {
    lightColor_ = color;
}

void PointLightNode::printChildrenList(std::stringstream & output) {
    output << "\n( P | " << depth_ << " ) " << name_;
    for (auto const& i : children_) {
        i->printChildrenList(output);
    }
}