#ifndef GEOMETRY_NODE_HPP
#define GEOMETRY_NODE_HPP

#include "node.hpp"
#include "model.hpp"

class GeometryNode:public Node {
    
    private:

        model model_;

    public:

        GeometryNode();
        GeometryNode(std::shared_ptr<model> model);
        ~GeometryNode();

        std::shared_ptr<model> getModel() const;
        void setModel(std::shared_ptr<model> model_ptr);

};

#endif