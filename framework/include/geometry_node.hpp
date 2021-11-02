#ifndef GEOMETRY_NODE_HPP
#define GEOMETRY_NODE_HPP

#include "node.hpp"
#include "model.hpp"

class GeometryNode : public Node {
    
    private:

        // member variables
        model model_;

    public:

        // con- & destructor
        GeometryNode();
        GeometryNode(   std::shared_ptr<Node> parent, 
                        std::vector<std::shared_ptr<Node>> children,
                        std::string const& name,
                        glm::mat4 const& localTransform,
                        std::shared_ptr<model> model );
        ~GeometryNode();

        // methods
        std::shared_ptr<model> getModel() const;
        void setModel(std::shared_ptr<model> model_ptr);

};

#endif