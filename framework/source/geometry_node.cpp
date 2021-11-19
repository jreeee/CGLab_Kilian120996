#include "geometry_node.hpp"

GeometryNode::GeometryNode():
    Node(), 
    model_ {nullptr} {
}

GeometryNode::GeometryNode( std::shared_ptr<Node> parent,
                            std::string const& name,
                            std::shared_ptr<model> model_ptr,
                            float spin,
                            float rotation):
    Node(parent, name),
    model_ {model_ptr},
    spin_ {spin},
    rotation_ {rotation} {
}

GeometryNode::~GeometryNode() {
    model_ = nullptr;
}

std::shared_ptr<model> GeometryNode::getModel() const {
    return model_;
}

void GeometryNode::setModel(std::shared_ptr<model> model_ptr) {
    model_ = model_ptr;
}

void GeometryNode::printChildrenList(std::stringstream & output) {
        output << "\n( G | " << depth_ << " ) " << name_;
    for (auto const& i : children_) {
        i->printChildrenList(output);
    }
}

float GeometryNode::getSpin() const {
    return spin_;
}

float GeometryNode::getRot() const {
    return rotation_;
}