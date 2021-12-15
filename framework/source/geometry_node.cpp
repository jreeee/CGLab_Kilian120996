#include "geometry_node.hpp"

GeometryNode::GeometryNode():
    Node(), 
    model_ {nullptr},
    material_ {nullptr} {
}

GeometryNode::GeometryNode( std::shared_ptr<Node> parent,
                            std::string const& name,
                            float distance,
                            std::shared_ptr<Material> material ):
    Node(parent, name),
    model_ {nullptr},
    spin_ {0.0f},
    rotation_ {0.0f},
    distance_ {distance},
    size_ {1.0f},
    material_ {material} {
}

GeometryNode::GeometryNode( std::shared_ptr<Node> parent,
                            std::string const& name,
                            std::shared_ptr<model> model_ptr,
                            float spin,
                            float rotation,
                            float distance,
                            float size,
                            std::shared_ptr<Material> material ):
    Node(parent, name),
    model_ {model_ptr},
    spin_ {spin},
    rotation_ {rotation},
    distance_ {distance},
    size_ {size},
    material_ {material} {
}

GeometryNode::~GeometryNode() {
    model_ = nullptr;
    material_= nullptr;
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

std::shared_ptr<Material> GeometryNode::getMaterial() const {
    return material_;
}

void GeometryNode::setMaterial(std::shared_ptr<Material> material) {
    material_ = material;
}

void GeometryNode::setTexId(unsigned int id, unsigned int tex_id) {
    tex_id_[id] = tex_id;
}

unsigned int GeometryNode::getTexId(unsigned int id) const {
    return tex_id_[id];
}


void GeometryNode::setNormal(bool normal) {
    normal_ = normal;
}
bool GeometryNode::hasNormal() const {
    return normal_;
}