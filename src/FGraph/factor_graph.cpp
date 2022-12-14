/* Copyright (c) 2022, Gonzalo Ferrer
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 *
 * factor_graph.cpp
 *
 *  Created on: Feb 12, 2018
 *      Author: Gonzalo Ferrer
 *              g.ferrer@skoltech.ru
 *              Mobile Robotics Lab, Skoltech 
 */

#include <iostream>
#include <mrob/factor_graph.hpp>


using namespace mrob;

FGraph::FGraph() :
        stateDim_(0),obsDim_(0)
{
}
FGraph::~FGraph()
{
    factors_.clear();
    nodes_.clear();
    eigen_factors_.clear();
}

factor_id_t FGraph::add_factor(std::shared_ptr<Factor> &factor)
{
    factor->set_id(factors_.size());
    factors_.emplace_back(factor);
    obsDim_ += factor->get_dim_obs();
    return factor->get_id();
}

factor_id_t FGraph::add_eigen_factor(std::shared_ptr<EigenFactor> &factor)
{
    factor->set_id(eigen_factors_.size());
    eigen_factors_.emplace_back(factor);
    return factor->get_id();
}


factor_id_t FGraph::add_node(std::shared_ptr<Node> &node)
{
	node->set_id(nodes_.size());
	nodes_.emplace_back(node);
	switch(node->get_node_mode())
	{
	    case Node::nodeMode::STANDARD:
	        active_nodes_.push_back(node);
	        stateDim_ += node->get_dim();
	        break;
	    case Node::nodeMode::ANCHOR:
	        break;
	    case Node::nodeMode::SCHUR_MARGI:
	        assert(0 && "add_node::SCHUR_MARGI: Functionality not programmed yey");
	        break;
	}
	return node->get_id();
}

std::shared_ptr<Node>& FGraph::get_node(factor_id_t key)
{
    assert(key < nodes_.size() && "FGraph::get_node: incorrect key");
    return nodes_[key];
}

std::shared_ptr<Factor>& FGraph::get_factor(factor_id_t key)
{
    // TODO key on a set or map?
    assert(key < factors_.size() && "FGraph::get_factor: incorrect key");
    return factors_[key];
}

std::shared_ptr<EigenFactor>& FGraph::get_eigen_factor(factor_id_t key)
{
    assert(key < eigen_factors_.size() && "FGraph::get_eigen_factor: incorrect key");
    return eigen_factors_[key];
}

void FGraph::print(bool completePrint) const
{
    std::cout << "Status of graph: " <<
            " Nodes = " << nodes_.size()  <<
            ", Factors = " << factors_.size() <<
            ", Eigen Factors = " << eigen_factors_.size() << std::endl;

    if(completePrint)
    {
        for (auto &&n : nodes_)
            n->print();
        for (auto &&f : factors_)
            f->print();
        for (auto &&f : eigen_factors_)
            f->print();
    }
}
