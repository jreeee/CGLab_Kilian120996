#ifndef POINTLIGHT_NODE_HPP
#define POINTLIGHT_NODE_HPP

#include "node.hpp"

class PointLightNode : public Node {

    private:

        float lightIntensity_;
        glm::vec3 lightColor_;

    public:

        PointLightNode();
        PointLightNode( std::shared_ptr<Node> parent,
                        std::string const& name,
                        glm::vec3 const& lightColor,
                        float lightIntensity);
        ~PointLightNode();

        float getIntensity() const;
        void setIntensity(float intensity);
        glm::vec3 getLightColor() const;
        void setLightColor(glm::vec3 const& lightColor);
        void printChildrenList(std::stringstream & output) override;
};

#endif