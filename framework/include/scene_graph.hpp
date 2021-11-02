#ifndef SCENE_GRAPH_HPP
#define SCENE_GRAPH_HPP

#include <string>

#include "node.hpp"

class SceneGraph {

    private:

        std::string name_;
        Node root_;

    public:

        //constructor
        SceneGraph(Node const& root);
        SceneGraph(std::string const& name, Node const& root);
        //destructor? virtual ~SceneGraph();
        
        std::string getName();
        void setName(std::string const& name);
        Node getRoot();
        void setRoot(Node const& root);
        std::string printGraph();

};

#endif