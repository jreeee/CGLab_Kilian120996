#include "scene_graph.hpp"

//TODO Singleton

// constructors
SceneGraph::SceneGraph():
    name_ {"default scene graph"},
    root_ {nullptr}, 
    speed_ {1.0f} {
}

SceneGraph::SceneGraph(std::string const& name, std::shared_ptr<Node> root):
    name_ {name},
    root_ {root},
    speed_ {1.0f} {
}

SceneGraph::~SceneGraph() {
    root_ = nullptr;
}

// methods
std::string SceneGraph::getName() const {
    return name_;
}

void SceneGraph::setName(std::string const& name) {
    name_ = name;
}

std::shared_ptr<Node> SceneGraph::getRoot() const {
    return root_;
}

void SceneGraph::setRoot(std::shared_ptr<Node> root) {
    root_ = root;
}

float SceneGraph::getSpeed() const {
    return speed_;
}

void SceneGraph::setSpeed(float speed) {
    speed_ = speed;
}

std::string SceneGraph::printGraph() {

    //String with the basic information, the rest will be appended
    std::stringstream graph;
    std::string info = "\nScene Graph of " + name_ + "\n\n"
                        + "( Type | Depth ) Node Name:\n-----------------\n"
                        + "( R | " + std::to_string(root_->getDepth()) 
                        + " ) " + root_->getName();
    graph << info;
    for(auto i : root_->getChildrenList()) {
        // printChildrenList() will recursively print all Nodes in the SceneGraph
        i->printChildrenList(graph);
    }
    graph << std::endl;
    return graph.str();
}

std::shared_ptr<CameraNode> SceneGraph::getCamera() const {
    //TODO recursion
    for(auto i : root_->getChildrenList()) {
        auto camera = std::dynamic_pointer_cast<CameraNode>(i);
        if (camera && camera->getEnabled()) return camera;
    }
    return nullptr;
}