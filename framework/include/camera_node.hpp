#ifndef CAMERA_NODE_HPP
#define CAMERA_NODE_HPP

#include "node.hpp"

class CameraNode:public Node {

    private:

        bool isPrespective_;
        bool isEnabled_;
        glm::mat4 projectionMatrix_;

    public:

        bool getPerspective() const;
        bool getEnabled() const;
        void setEnabled(bool enable);
        glm::mat4 getProjectionMatrix() const;
        void setProjectionMatrix(glm::mat4 const& projectionMatrix);

};

#endif