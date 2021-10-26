#ifndef NODE_HPP
#define NODE_HPP

#include <list>
#include <glm>

class Node {

    Node parent;
    std::list<Node> children;
    std::string name;
    std::string path;
    int depth;
    glm::mat4 localTransform;
    glm::mat4 worldTransform;
    

}

#endif