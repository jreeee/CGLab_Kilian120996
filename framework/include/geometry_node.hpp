#ifndef GEOMETRY_NODE_HPP
#define GEOMETRY_NODE_HPP

#include "node.hpp"
#include "model.hpp"

class GeometryNode : public Node {
    
    private:

        // member variables
        std::shared_ptr<model> model_;
        //spin is the rotation around the sun, rotation is the rotation around itself
        float spin_;
        float rotation_;

    public:

        // con- & destructor
        GeometryNode();
        GeometryNode(   std::shared_ptr<Node> parent, 
                        std::string const& name,
                        std::shared_ptr<model> model_ptr,
                        float spin,
                        float rotation );
        ~GeometryNode();

        // methods
        std::shared_ptr<model> getModel() const;
        void setModel(std::shared_ptr<model> model_ptr);
        
        void printChildrenList(std::stringstream & output) override;
        float getSpin() const;
        float getRot() const;
};

#endif