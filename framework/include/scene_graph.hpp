#ifndef SCENE_GRAPH_HPP
#define SCENE_GRAPH_HPP

#include <string>

#include "node.hpp"

class SceneGraph {

    private:

        std::string name_;
        std::shared_ptr<Node> root_;

    public:

        //constructor
        SceneGraph();
        SceneGraph(std::string const& name, std::shared_ptr<Node> root);
        //destructor? virtual ~SceneGraph();
        
        std::string getName() const;
        void setName(std::string const& name);
        std::shared_ptr<Node> getRoot() const;
        void setRoot(std::shared_ptr<Node> root);
        std::string printGraph() const;

};

#endif