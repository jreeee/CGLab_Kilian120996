#ifndef POINTLIGHT_NODE_HPP
#define POINTLIGHT_NODE_HPP

#include "node.hpp"
#include "structs.hpp"

class PointLightNode : public Node {

    private:

        float lightIntensity_;
        Color lightColor_;

    public:

        PointLightNode();
        PointLightNode( std::shared_ptr<Node> parent,
                        std::string const& name,
                        Color const& lightColor,
                        float lightIntensity);
        ~PointLightNode();

        float getIntensity() const;
        void setIntensity(float intensity);
        Color getLightColor() const;
        void setLightColor(Color const& lightColor);
        void printChildrenList(std::stringstream & output) override;
};

#endif