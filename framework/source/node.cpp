#include "node.hpp"

Node::Node():
    parent {nullptr},
    children {std::list<std::shared_ptr<Node>>},
    name {"default"},
    path {""},
    depth {-1},
    localTransform {glm::mat4 (1.0)},
    worldTransform {glm::mat4 (1.0)} {

}

