#ifndef CAMERA_NODE_HPP
#define CAMERA_NODE_HPP

#include "node.hpp"

class CameraNode : public Node {

    private:

        // member variables
        bool isPrespective_;
        bool isEnabled_;
        glm::mat4 projectionMatrix_;

    public:
        
        // con- & destructor
        CameraNode();
        CameraNode( std::shared_ptr<Node> parent, 
                    std::vector<std::shared_ptr<Node>> children,
                    std::string const& name,
                    glm::mat4 const& localTransform,
                    bool perspective, 
                    bool enabled, 
                    glm::mat4 const& projectionMatrix );
        ~CameraNode();

        // methods
        bool getPerspective() const;
        bool getEnabled() const;
        void setEnabled(bool enable);
        glm::mat4 getProjectionMatrix() const;
        void setProjectionMatrix(glm::mat4 const& projectionMatrix);

};

#endif