#ifndef GEOMETRY_NODE_HPP
#define GEOMETRY_NODE_HPP

#include "node.hpp"
#include "model.hpp"

class GeometryNode : public Node {
    
    private:

        // member variables
        std::shared_ptr<model> model_;

    public:

        // con- & destructor
        GeometryNode();
        GeometryNode(   std::shared_ptr<Node> parent, 
                        std::string const& name,
                        std::shared_ptr<model> model_ptr );
        ~GeometryNode();

        // methods
        std::shared_ptr<model> getModel() const;
        void setModel(std::shared_ptr<model> model_ptr);
        
        void printChildrenList(std::stringstream & output) override;

};

#endif