#include "scene_graph.hpp"

//TODO Singleton

// constructors
SceneGraph::SceneGraph():
    name_ {"default scene graph"},
    root_ {nullptr} {
}

SceneGraph::SceneGraph(std::string const& name, std::shared_ptr<Node> root):
    name_ {name},
    root_ {root} {
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

std::string SceneGraph::printGraph() {

    //String with the basic information, the rest will be appended
    std::stringstream graph;
    std::string info = "Scene Graph of " + name_ + "\n"
                        + "Node Name, ( Type | Depth ):\n."
                        + root_->getName() + "(R|" 
                        + std::to_string(root_->getDepth()) + ")\n";
    graph << info;
    for(auto i : root_->getChildrenList()){
        // printChildrenList() will recursively print all Nodes in the SceneGraph
        i->printChildrenList(graph);
    }
    return graph.str();
}