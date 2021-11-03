#include "camera_node.hpp"

CameraNode::CameraNode():
    Node(),
    isPrespective_ {true},
    isEnabled_ {false},
    projectionMatrix_ {glm::mat4()} {

}

CameraNode::CameraNode( std::shared_ptr<Node> parent, 
                    std::vector<std::shared_ptr<Node>> children,
                    std::string const& name,
                    glm::mat4 const& localTransform,
                    bool perspective, 
                    bool enabled, 
                    glm::mat4 const& projectionMatrix ):
    Node(parent, children, name, localTransform),
    isPrespective_ {perspective},
    isEnabled_ {enabled},
    projectionMatrix_ {projectionMatrix} {

}

bool CameraNode::getPerspective() const {
    return isPrespective_;
}

void CameraNode::setPerspective(bool enable) {
    isPrespective_ = enable;
}

bool CameraNode::getEnabled() const {
    return isEnabled_;
}

void CameraNode::setEnabled(bool enable) {
    isEnabled_ = enable;
}

glm::mat4 CameraNode::getProjectionMatrix() const {
    return projectionMatrix_;
}

void CameraNode::setProjectionMatrix(glm::mat4 const& projectionMatrix) {
    projectionMatrix_ = projectionMatrix;
}

void CameraNode::printChildrenList(std::stringstream & output) {
    output << name_ << " \t(C|" << depth_ << ")\n";
    for (auto i : children_) {
        i->printChildrenList(output);
    }
}