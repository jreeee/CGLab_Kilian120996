#ifndef POINTLIGHT_NODE_HPP
#define POINTLIGHT_NODE_HPP

#include "node.hpp"

class PointLightNode : public Node {

    private:

        float intensity_;

    public:

        PointLightNode();
        PointLightNode( std::shared_ptr<Node> parent,
                        std::string const& name, 
                        float intensity);
        ~PointLightNode();

        float getIntensity() const;
        void setIntensity(float intensity);
        void printChildrenList(std::stringstream & output) override;
};

#endif