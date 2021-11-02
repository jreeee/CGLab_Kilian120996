#include "node.hpp"


// constructors
Node::Node():
    parent_ {nullptr},
    children_ {std::vector<std::shared_ptr<Node>>()},
    name_ {"default"},
    path_ {""},
    depth_ {-1},
    localTransform_ {glm::mat4 (1.0)},
    worldTransform_ {glm::mat4 (1.0)} { 
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
    depth_ = -1;
    //depth_ = parent->getDepth() + 1;
    path_ = "todo";
    //path_ = parent->getPath() + name_;
    worldTransform_ = localTransform; //1 should be parent->worldTransform
}

// methods
