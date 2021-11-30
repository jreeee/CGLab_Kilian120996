#include "geometry_node.hpp"

GeometryNode::GeometryNode():
    Node(), 
    model_ {nullptr},
    color_ {nullptr} {
}

GeometryNode::GeometryNode( std::shared_ptr<Node> parent,
                            std::string const& name,
                            float distance,
                            std::shared_ptr<Color> color ):
    Node(parent, name),
    model_ {nullptr},
    spin_ {0.0f},
    rotation_ {0.0f},
    distance_ {distance},
    size_ {1.0f},
    color_ {color} {
}

GeometryNode::GeometryNode( std::shared_ptr<Node> parent,
                            std::string const& name,
                            std::shared_ptr<model> model_ptr,
                            float spin,
                            float rotation,
                            float distance,
                            float size,
                            std::shared_ptr<Color> color ):
    Node(parent, name),
    model_ {model_ptr},
    spin_ {spin},
    rotation_ {rotation},
    distance_ {distance},
    size_ {size},
    color_ {color} {
}

GeometryNode::~GeometryNode() {
    model_ = nullptr;
    color_ = nullptr;
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

float GeometryNode::getDist() const {
    return distance_;
}

float GeometryNode::getSize() const {
    return size_;
}

std::shared_ptr<Color> GeometryNode::getColor() const {
    return color_;
}

void GeometryNode::setColor(std::shared_ptr<Color> color) {
    color_ = color;
}