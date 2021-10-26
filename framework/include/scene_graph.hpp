#ifndef SCENE_GRAPH_HPP
#define SCENE_GRAPH_HPP

#include <std::string>

#include "node.hpp"

class SceneGraph {

    std::string Name;
    Node root;
    //constructor
    SceneGraph(Node const& root);
    SceneGraph(std::string const& name, Node const& root);
    //destructor? virtual ~SceneGraph();
    
    std::string getName();
    void setName(std::string const& name);
    Node getRoot();
    void setRoot(Node const& root);
    std::string printGraph();

}

#endif