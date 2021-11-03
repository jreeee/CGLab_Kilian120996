#include "geometry_node.hpp"

GeometryNode::GeometryNode():
    Node(), 
    model_ {nullptr} {
}

GeometryNode::GeometryNode( std::shared_ptr<Node> parent,
                            std::string const& name,
                            std::shared_ptr<model> model_ptr ):
    Node(parent, name),
    model_ {model_ptr} {
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
    output << name_ << " \t(G|" << depth_ << ")\n";
    for (auto i : children_) {
        i->printChildrenList(output);
    }
}