#ifndef GEOMETRY_NODE_HPP
#define GEOMETRY_NODE_HPP

#include "node.hpp"
#include "model.hpp"
#include "structs.hpp"

class GeometryNode : public Node {
    
    private:

        // member variables
        std::shared_ptr<model> model_;
        //spin is the rotation around the sun, rotation is the rotation around itself
        float spin_;
        float rotation_;
        float distance_;
        float size_;
        std::shared_ptr<Color> color_;

    public:

        // con- & destructor
        GeometryNode();
        //for orbits
        GeometryNode(   std::shared_ptr<Node> parent, 
                        std::string const& name,
                        float distance,
                        std::shared_ptr<Color> color );
        //for planets
        GeometryNode(   std::shared_ptr<Node> parent, 
                        std::string const& name,
                        std::shared_ptr<model> model_ptr,
                        float spin,
                        float rotation,
                        float distance,
                        float size,
                        std::shared_ptr<Color> color );
        ~GeometryNode();

        // methods
        std::shared_ptr<model> getModel() const;
        void setModel(std::shared_ptr<model> model_ptr);
        
        void printChildrenList(std::stringstream & output) override;
        float getSpin() const;
        float getRot() const;
        float getDist() const;
        float getSize() const;
        std::shared_ptr<Color> getColor() const;
        void setColor(std::shared_ptr<Color> color);

};

#endif