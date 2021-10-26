#ifndef NODE_HPP
#define NODE_HPP

#include <list>
#include <string>
#include <memory>
#include <glm/glm.hpp>

class Node {

    std::shared_ptr<Node> parent;
    std::list<std::shared_ptr<Node>> children;
    std::string name;
    std::string path;
    int depth;
    glm::mat4 localTransform;
    glm::mat4 worldTransform;

    Node();

    virtual ~Node();

};

#endif