#include "node.hpp"


// constructors
Node::Node():
    parent_ {nullptr},
    children_ {std::vector<std::shared_ptr<Node>>()},
    name_ {"default"},
    path_ {""},
    depth_ {-1},
    localTransform_ {glm::mat4()},
    worldTransform_ {glm::mat4()} { 
}

// TDOD generate path, depth
Node::Node( std::shared_ptr<Node> parent, 
            std::vector<std::shared_ptr<Node>> children,
            std::string const& name,
            glm::mat4 const& localTransform ):
    parent_ {parent},
    children_ {children},
    name_ {name},
    localTransform_ {localTransform}
{
    depth_ = 0;
    path_ = "";
    worldTransform_ = localTransform;

    if (parent_ != nullptr) {
        depth_ = parent_->depth_ + 1;
        path_ = parent_->path_ + "/" + name_;
        worldTransform_ = localTransform * parent_->worldTransform_;
    }
    else {
        path_ = "/" + name_;
    }
}

Node::~Node(){
    parent_ = nullptr;
    children_.clear();
}

// methods
std::shared_ptr<Node> Node::getParent() const {
    return parent_;
}

void Node::setParent(std::shared_ptr<Node> parent) {
    parent_ = parent;
}

std::shared_ptr<Node> Node::getChildren(std::string const& name) const {
    for (auto i : children_) {
        if (i->getName() == name) {
            return i;
        }
    }
    return nullptr;
}

std::vector<std::shared_ptr<Node>> Node::getChildrenList() const {
    return children_;
}

std::string Node::getName() const {
    return name_;
}

std::string Node::getPath() const {
    return path_;
}

int Node::getDepth() const {
    return depth_;
}

glm::mat4 Node::getLocalTransform() const {
    return localTransform_;
}

void Node::setLocalTransform(glm::mat4 const& localTransform) {
    localTransform_ = localTransform;
}

glm::mat4 Node::getWorldTransform() const {
    return worldTransform_;
}

void Node::setWorldTransform(glm::mat4 const& worldTransform) {
    worldTransform_ = worldTransform;
}

void Node::addChildren(std::shared_ptr<Node> child) {
    if (child != nullptr) {
        children_.push_back(child);
    }
}

std::shared_ptr<Node> Node::removeChildren(std::string name) {
    std::shared_ptr<Node> child = getChildren(name);
    if (child != nullptr) {
        for (auto it = children_.begin(); it != children_.end(); ++it) {
            if ((*(*it)).getName() == name) {
                children_.erase(it);
                return child;
            }
        }
    }
    return nullptr;
}

void Node::printChildrenList(std::stringstream & output) {
    output << name_ << " \t(N|" << depth_ << ")\n";
    for (auto i : children_) {
        i->printChildrenList(output);
    }
}