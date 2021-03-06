#ifndef NODE_HPP
#define NODE_HPP

#include <vector>
#include <string>
#include <memory>
#include <glm/glm.hpp>
#include <sstream>

#include "node.hpp"

class Node {

    protected:

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
                std::string const& name);

        virtual ~Node();

        std::shared_ptr<Node> getParent() const;
        void setParent(std::shared_ptr<Node> parent);
        std::shared_ptr<Node> getChildren(std::string const& name) const;
        std::vector<std::shared_ptr<Node>> getChildrenList() const;
        std::string getName() const;
        std::string getPath() const;
        int getDepth() const;
        glm::mat4 getLocalTransform() const;
        void setLocalTransform(glm::mat4 const& localTransform);
        glm::mat4 getWorldTransform() const;
        void setWorldTransform(glm::mat4 const& worldTransform);
        void addChildren(std::shared_ptr<Node> child);
        std::shared_ptr<Node> removeChildren(std::string name);

        //can be overwritten by the other node-subclasses
        virtual void printChildrenList(std::stringstream & output);
};

#endif