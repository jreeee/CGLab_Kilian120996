#include "node.hpp"


// constructors
Node::Node():
    parent_ {nullptr},
    children_ {std::vector<std::shared_ptr<Node>>{}},
    name_ {"default"},
    path_ {""},
    depth_ {-1},
    localTransform_ {glm::mat4()},
    worldTransform_ {glm::mat4()} { 
}

// TDOD generate path, depth
Node::Node( std::shared_ptr<Node> parent, 
            std::vector<std::shared_ptr<Node>> const& children,
            std::string const& name,
            glm::mat4 const& localTransform ):
    children_ {children},
    name_ {name},
    localTransform_ {localTransform}
{
    setParent(parent);
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
    if (parent_ == nullptr) {
        path_ = "/" + name_;
        depth_ = 0;
        worldTransform_ = localTransform_ = glm::mat4{};
    }
    else {
        path_ = parent_->path_ + "/" + name_;
        depth_ = parent_->depth_ + 1;
        worldTransform_ = parent_->worldTransform_ * localTransform_;

        //update the parent, so that it also has a reference to the node
        //doesn't work sadly
        //auto ptr = std::make_shared<Node>(*this);
    }
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
    // it'd be bad if we were to add a Node with a lesser depth as child
    if (child != nullptr && child->depth_ > depth_) {
        //rn you can technically add the same node multiple times, stc.
        children_.push_back(child);
    }
}

std::shared_ptr<Node> Node::removeChildren(std::string name) {

    //preliminary check if a node with that name is in the vector
    std::shared_ptr<Node> child = getChildren(name);

    if (child != nullptr) {
        //iterating over said vector until we find the first child with that name
        for (auto it = children_.begin(); it != children_.end(); ++it) {
            if ((*(*it)).getName() == name) {
                //removing the pointer from the child to the parent
                (*(*it)).parent_ = nullptr;
                //removing the pointer fom the parent to the child
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
