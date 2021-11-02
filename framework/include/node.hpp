#ifndef NODE_HPP
#define NODE_HPP

#include <vector>
#include <string>
#include <memory>
#include <glm/glm.hpp>

class Node {

    private:

        std::shared_ptr<Node> parent_;
        std::vector<std::shared_ptr<Node>> children_;
        std::string name_;
        std::string path_;
        int depth_;
        glm::mat4 localTransform_;
        glm::mat4 worldTransform_;

    public:

        Node();
        Node(   std::shared_ptr<Node> parent, 
                std::vector<std::shared_ptr<Node>> children,
                std::string name,
                glm::mat4 localTransform,
                glm::mat4 worldTransform);

        virtual ~Node();

};

#endif