#include "scene_graph.hpp"

// constructors
SceneGraph::SceneGraph():
    name_ {"default scene graph"},
    root_ {nullptr} {
}

SceneGraph::SceneGraph(std::string const& name, std::shared_ptr<Node> root):
    name_ {name},
    root_ {root} {
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

std::string SceneGraph::printGraph() const {
    std::string graph = "Scene Graph of the " + name_ + "\n" ;
    for(auto i : root_->getChildrenList()){
        graph += i->printChildrenList();
    }
    return graph;
}