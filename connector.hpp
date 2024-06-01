/*
Copyright (c) 2024 Martin Werner Licht

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef IG_CONNECTOR
#define IG_CONNECTOR

#include <cassert>
#include <cmath>

#include <functional>
#include <iostream>
#include <limits>
#include <set>
#include <tuple>
#include <utility>
#include <vector>

#include "common.hpp"

#include "graph.hpp"
#include "grp.hpp"
#include "priority_queue.hpp"

struct BoundingBox
{
    int minx;
    int maxx;
    int miny;
    int maxy;
    int minz;
    int maxz;
};

std::ostream& operator<<(std::ostream& os, const BoundingBox& bb) {
    os << "BoundingBox(minx: " << bb.minx
       << ", maxx: " << bb.maxx
       << ", miny: " << bb.miny
       << ", maxy: " << bb.maxy
       << ", minz: " << bb.minz
       << ", maxz: " << bb.maxz << ")";
    return os;
}

class Connector {
private:
    const GlobalRoutingProblem& problem;
    const Graph& graph;
    
    std::vector<int> queued;
    std::vector<int> preceding_node;
    std::vector<int> relevant_edge;
    
    std::vector<float> distance;

    PriorityQueue<> pq;

    int current_iteration = -1;

    std::vector<int> aggregated_width;
    
public:
    static const int invalid_index;

    Connector( GlobalRoutingProblem& problem, Graph& graph );

    std::vector<std::set<int>> connect();

    std::set<int> create_search_forest( 
        const std::set<int>& S, const std::set<int>& T, 
        int min_net_width, 
        BoundingBox BB,
        bool respect_capcity, float capacity_penalty_factor = 10. );

};

const int Connector::invalid_index = -1;








bool verify_capacities( const GlobalRoutingProblem& grp, const Graph& graph, const std::vector<std::set<int>>& solutions, const std::vector<int>& aggregated_width )
{
    std::vector<int> remaining_capacities = graph.get_capacities();

    for( int e = 0; e < graph.count_edges(); e++ ) 
    {
        assert( remaining_capacities[e] == graph.get_capacity(e) );
        assert( remaining_capacities[e] >= 0 );
    } 

    for( int e = 0; e < graph.count_edges(); e++ )
    {
        assert( graph.get_capacity(e)   >= 0 );
        assert( aggregated_width[e]     >= 0 );
        assert( remaining_capacities[e] >= 0 );
        assert( aggregated_width[e] <= graph.get_capacity(e) );
    }

    for( int net_index = 0; net_index < solutions.size(); net_index++ )
    {
        const auto solution = solutions[net_index];
        
        for( const auto& e : solution )
        {
            const auto direction = graph.get_edge_direction(e);

            if( direction == Graph::direction::z_plus ) continue;

            const auto nodes = graph.get_nodes_of_edge( e );
            int x1, y1, z1, x2, y2, z2;
            std::tie( x1, y1, z1 ) = graph.get_position_from_nodeindex( nodes.first  );
            std::tie( x2, y2, z2 ) = graph.get_position_from_nodeindex( nodes.second );
    
            assert( z1 == z2 );
            assert( x1 == x2+1 or x1 == x2-1 or y1 == y2+1 or y1 == y2-1 );
            if( x1 != x2 ) assert( y1 == y2 );
            if( y1 != y2 ) assert( x1 == x2 );
            
            const auto min_spacing = grp.dimension.minimum_spacing[z1];
            const auto min_width   = grp.dimension.minimum_width[z1];
            
            const auto min_net_width = grp.nets[net_index].minimum_width;

            remaining_capacities[e] -= ( min_spacing + std::max(min_width,min_net_width) );

            assert( remaining_capacities[e] >= 0 );
        }
    }

    for( int e = 0; e < graph.count_edges(); e++ ) {
        assert( remaining_capacities[e] + aggregated_width[e] == graph.get_capacity(e) );
        assert( remaining_capacities[e] >= 0 );
    } 

    return true;
}


bool verify_connector( const GlobalRoutingProblem& grp, const Graph& graph, int net_index, const std::set<int> targets, const std::set<int>& edgeindices )
{
    std::set<int> nodes;


    // for( const auto edgeindex : edgeindices )
    // {
    //     const auto edge = graph.get_nodes_of_edge( edgeindex );
    //     printf("(%d,%d) ", edge.first, edge.second );
    // }
    // printf("\n");

            
    // if no edges, check that it is all on the same tile 
    if( edgeindices.empty() )
    {
        const auto& pins = grp.nets[net_index].pins;

        if( pins.empty() ) return true;

        const auto s = pins.begin();
        int sx, sy;
        std::tie( sx, sy ) =  grp.tile_of_coordinate( s->x, s->y );

        for( const auto p : pins )
        {
            int tx, ty;
            std::tie( tx, ty ) =  grp.tile_of_coordinate( p.x, p.y );
        
            assert( sx == tx && sy == ty );
        };

        return true;
    }

    // get all the node indices of the edges 
    for( int edgeindex : edgeindices )
    {
        const auto edge = graph.get_nodes_of_edge( edgeindex );

        nodes.insert( edge.first );
        nodes.insert( edge.second );
    }

    const auto& pins = grp.nets[net_index].pins;

    // std::clog << "N/E/P " << nodes.size() << ' ' << edgeindices.size() << ' ' << pins.size() << std::endl;

    // check that each pin is contained in the set of nodes 
    for( const auto p : pins )
    {
        int tx, ty;
        std::tie( tx, ty ) =  grp.tile_of_coordinate( p.x, p.y );

        int pin_node = graph.get_nodeindex_from_position( tx, ty, p.layer );

        assert( nodes.contains( pin_node ) );
    };

    

    // check that the graph is not cyclic 
    {
        std::set<int> nodes_visited;

        // pick some arbitrary node
        int first_node = *(nodes.begin());

        std::function<void(int,int)> 
        // recursive_check();
        
        recursive_check = [&]( int current_node, int parent_node ) -> void {

            // std::clog << current_node << ' ' << parent_node << '\n';

            assert( not nodes_visited.contains(current_node) );

            nodes_visited.insert( current_node );

            const auto& adjacent_edges = graph.get_edgeindices_from_node( current_node );

            int count_relevant_edges = 0;

            for( int edge_index : adjacent_edges )
            {
                if( not edgeindices.contains( edge_index ) ) continue;

                const auto nodes_of_edge = graph.get_nodes_of_edge( edge_index );

                int other_node = ( nodes_of_edge.first == current_node ) ? nodes_of_edge.second : nodes_of_edge.first;
                assert( nodes_of_edge.first == current_node || nodes_of_edge.second == current_node );
                assert( other_node != current_node );
                
                if( other_node == parent_node ) continue;

                recursive_check( other_node, current_node );

                count_relevant_edges++;
            }

            // if no neighboring node except (possibly) parent, then we are in a terminal node 
            if( count_relevant_edges == 0 ) assert( targets.contains( current_node ) );

            // if happen to start at a terminal node, check there is at most one other edge 
            if( parent_node == -1 && count_relevant_edges <= 1 ) assert( targets.contains( current_node ) );

        };

        recursive_check( first_node, -1 );

        std::set<int> all_nodes;
        // collect all nodes in the edges 
        for( const auto e : edgeindices )
        {
            const auto nodes = graph.get_nodes_of_edge(e);
            all_nodes.insert( nodes.first );
            all_nodes.insert( nodes.second );
        }

        assert( all_nodes == nodes_visited );

        assert( all_nodes.size() == edgeindices.size() + 1 );

    }
    
    
    // check that the graph is connected
    if(false)
    { 
        // we run over all the edges 
        // make a queue of all edges and begin with any edge, and keep track of all edges visited 
        
        std::set<int> edges_queued;
        edges_queued.insert( *(edgeindices.begin()) );

        std::set<int> edges_checked;
        
        while( ! edges_queued.empty() )
        {
            int edgeindex = *(edges_queued.begin());
            assert( edges_queued.contains(edgeindex) );
            edges_queued.erase(edgeindex);
            assert( not edges_queued.contains(edgeindex) );

            // std::clog << "check " << edgeindex << '\n';

            assert( edgeindices.contains(edgeindex) );

            // assert the popped edge is not contained twice 

            if( edges_queued.contains(edgeindex) )
            {
                for( const auto e : edges_queued ) 
                    std::clog << e << std::endl;
                std::clog << edgeindex << std::endl;
            }
            

            assert( not edges_queued.contains(edgeindex) );

            // assert this edge has not been visited yet; every edge is visited only once 
            // then insert in the set of checked edges  
            assert( not edges_checked.contains(edgeindex) );
            edges_checked.insert(edgeindex);

            // collect the two nodes and extract the adjacent edges 
            int nodes[2];
            std::tie( nodes[0], nodes[1] ) = graph.get_nodes_of_edge(edgeindex);

            const std::vector<int> adjacent_edges[2] = { graph.get_edgeindices_from_node( nodes[0] ), graph.get_edgeindices_from_node( nodes[1] ) };
            
            // for each adjacent edge
            // - skip if not in the tree or if already visited
            for( const auto& edges : adjacent_edges )
            for( const int other_edge : edges )
            {
                // we skip if the current edge anyways
                if( other_edge == edgeindex ) continue;
                
                // we skip what has already been checked 
                if( edges_checked.contains(other_edge) ) continue;
                
                // we skip edges that are not in the subgraph
                if( not edgeindices.contains(other_edge) ) continue;

                // we skip edges that are already queued (because they were added when the preceding edge was processed)
                if( edges_queued.contains( other_edge ) ) continue;

                assert( not edges_queued.contains( other_edge ) );

                edges_queued.insert( other_edge );

                // edges_checked.insert(other_edge);

                assert( edges_queued.size() <= edgeindices.size() );
            };
            
        }
        assert( edges_queued.empty() );
    
    }
    
    return true;
}














Connector::Connector( GlobalRoutingProblem& problem, Graph& graph )
: 
problem( problem ), 
graph( graph ), 
queued( graph.count_nodes(), -1 ),
preceding_node( graph.count_nodes(), -1 ),
relevant_edge( graph.count_nodes(), -1 ),
distance( graph.count_nodes(), std::numeric_limits<float>::quiet_NaN() ),
current_iteration( 0 ),
aggregated_width( graph.count_edges(), 0 )
{
}






std::vector<std::set<int>> Connector::connect()
{
    std::vector<std::set<int>> trees( problem.nets.size() );

    // For each net 
    for( int n = 0; n < problem.nets.size(); n++ ) 
    {
        const auto& net = problem.nets[n];
        
        // list the tiles in the net 
        const auto pins = net.pins;

        // if there are no pins, then skip 
        if( pins.size() == 0 ) continue;

        std::clog << "Routing net\t " << n << "/" << problem.nets.size() << "\t pins: " << pins.size() << "\n";

        std::vector<int> nodes;
        nodes.reserve( pins.size() );
        
        for( const auto& pin : pins )
        {
            std::pair<int,int> tile_xy = problem.tile_of_coordinate( pin.x, pin.y );

            int nodeindex = graph.get_nodeindex_from_position( tile_xy.first, tile_xy.second, pin.layer );

            nodes.push_back( nodeindex );
        }

        {
            std::sort(nodes.begin(), nodes.end());
            auto last = std::unique(nodes.begin(), nodes.end());
            nodes.erase(last, nodes.end());
        }

        // Bounding box 
        BoundingBox BB = {
            problem.grid.x_grids, 0,
            problem.grid.y_grids, 0,
            problem.grid.layers,  0,
        };

        for( auto nodeindex : nodes )
        {
            int x,y,z;
            std::tie(x,y,z) = graph.get_position_from_nodeindex(nodeindex);

            BB.maxx = std::max( x, BB.maxx );
            BB.minx = std::min( x, BB.minx );
            
            BB.maxy = std::max( y, BB.maxy );
            BB.miny = std::min( y, BB.miny );
            
            BB.minz = std::min( z, BB.minz );
            BB.maxz = std::max( z, BB.maxz );
            
            assert( not( BB.minx > x or BB.maxx < x or BB.miny > y or BB.maxy < y or BB.minz > z or BB.maxz < z ) );
        }

        BB.maxx = std::min(BB.maxx + 10, problem.grid.x_grids-1);
        BB.maxy = std::min(BB.maxy + 10, problem.grid.y_grids-1);
        BB.maxz = std::min(BB.maxz + 10, problem.grid.layers-1);
        
        BB.minx = std::max(BB.minx - 10, 0);
        BB.miny = std::max(BB.miny - 10, 0);
        BB.minz = std::max(BB.minz - 10, 0);
        
        assert( 0 <= BB.minx and BB.minx <= BB.maxx and BB.maxx < problem.grid.x_grids );
        assert( 0 <= BB.miny and BB.miny <= BB.maxy and BB.maxy < problem.grid.y_grids );
        assert( 0 <= BB.minz and BB.minz <= BB.maxz and BB.maxz < problem.grid.layers  );
        
        // having collected all nodes, separate them into S and T

        std::set<int> S; 
        std::set<int> T; 

        // int random_index = rand() % nodes.size();

        int random_index = 0; // we assume that the pins are ordered to that the first one is at the center

        for( int i = 0; i < nodes.size(); i++ )
        {
            if( random_index == i )
                S.insert( nodes[i] ); 
            else 
                T.insert( nodes[i] );
        }

        // std::clog << nodes.size() <<' '<< random_index <<' '<< S.size() <<' '<< T.size() << '\n';
        assert( S.size() + T.size() == nodes.size() );
        assert( S.size() == 1 );
        
        // create the Steiner tree 

        int min_net_width = problem.nets[n].minimum_width;

        const auto edgeindices = create_search_forest( S, T, min_net_width, BB, true );

        trees[n] = edgeindices;

        auto node_set = T; 
        node_set.merge(S);

        assert( verify_connector( problem, graph, n, node_set, edgeindices ) );

        // update the aggregated widths 
        for( const auto edgeindex : edgeindices )
        {
            const auto edge_orientation = graph.get_edge_direction( edgeindex );

            if( edge_orientation == Graph::direction::z_plus ) continue;

            auto nodes = graph.get_nodes_of_edge( edgeindex );
            
            int x1,y1,z1;
            int x2,y2,z2;
            std::tie( x1,y1,z1 ) = graph.get_position_from_nodeindex( nodes.first  );
            std::tie( x2,y2,z2 ) = graph.get_position_from_nodeindex( nodes.second );
            
            assert( z1 == z2 );
            assert( x1 == x2+1 or x1 == x2-1 or y1 == y2+1 or y1 == y2-1 );
            if( x1 != x2 ) assert( y1 == y2 );
            if( y1 != y2 ) assert( x1 == x2 );
            
            int required_capacity = std::max( min_net_width, problem.dimension.minimum_width[z1] ) + problem.dimension.minimum_spacing[z1];
            
            assert( required_capacity >= 0 );
            
            assert( aggregated_width[edgeindex] >= 0 );
            
            // assert( aggregated_width[edgeindex] + required_capacity <= graph.get_capacity(edgeindex) );

            aggregated_width[edgeindex] += required_capacity;

            assert( aggregated_width[edgeindex] >= 0 );
            
            // assert( aggregated_width[edgeindex] <= graph.get_capacity( edgeindex ) );
        }
    
    }

    // assert( verify_capacities( problem, graph, trees, aggregated_width ) );

    return trees;
}






std::set<int> Connector::create_search_forest( 
    const std::set<int>& S, const std::set<int>& T, 
    int min_net_width, 
    BoundingBox BB,
    bool respect_capacity, 
    float capacity_penalty_factor )
{
    assert( capacity_penalty_factor >= 0. and min_net_width >= 0 );
    
    // prepare this set to be returned 
    std::set<int> ret; 

    // clear every possible leftover from the previous iteration 
    pq.clear();

    // Increase iteration counter 
    current_iteration++;
    assert( current_iteration >= 0 );

    std::clog << "BB: " << BB.maxx - BB.minx << tab << BB.maxy - BB.miny << tab << BB.maxz - BB.minz << nl;
    std::clog << "PQ capacity (start): " << pq.capacity() << std::endl;
    assert( pq.size() == 0 );

    // enter all source nodes into the queue
    // they are part of the current iteration, have no preceding node, and distance 0
    for( int s : S )
    {
        assert( 0 <= s && s < graph.count_nodes() );
        pq.push( s, 0. );
        queued[s]        = current_iteration;
        preceding_node[s] = -1;
        relevant_edge[s] = -1;
        distance[s]       = 0.;
    }

    auto active_T = T;

    float last_distance = 0.; // TODO here a dummy variable to check that distances keep increasing 
    int max_pq_size = 0;
    int num_iterations = 0;

    if( respect_capacity )
    for( auto nodeindex : active_T )
    {
        int x, y, z;
        std::tie(x,y,z) = graph.get_position_from_nodeindex( nodeindex );
        if( not( BB.minx > x or BB.maxx < x or BB.miny > y or BB.maxy < y or BB.minz > z or BB.maxz < z ) ) 
        continue;
        clog << "Terminal outside of box:\n";
        std::clog << BB << nl;
        std::clog << x << tab << y << tab << z << nl;
    }
                    

    // keep searching as long as T is not empty, that is, not all targets have been found 
    while( not active_T.empty() )
    {
        max_pq_size = std::max( max_pq_size, pq.size() );

        num_iterations++;
        
        // if active_T still contains nodes but the PQ is empty,
        // then the graph is too congested to reach the remaining terminals.
        // We enter unrestricted mode 
        if( not respect_capacity ) assert( not pq.empty() );

        if( pq.empty() ){
            assert( respect_capacity );
            std::clog << "EMERGENCY MODE" << nl;
            return create_search_forest( S, T, min_net_width, BB, false, capacity_penalty_factor );
        }

        // get priority node and its distance 
        PriorityQueue<>::Entry current_entry = pq.pop();

        int current_node     = current_entry.value;
        
        float current_distance = current_entry.priority;

        assert( std::isfinite( current_distance ) && std::isfinite( distance[current_node] ) );
        assert( current_distance == distance[current_node] );

        // TODO: check that distance has increased 
        assert( last_distance <= current_distance ); last_distance = current_distance;
        
        // get all edges at that node 
        std::vector<int> edges = graph.get_edgeindices_from_node( current_node );

        // iterate over all edges 
        for( const auto edgeindex : edges )
        {

            const auto current_edge = graph.get_nodes_of_edge( edgeindex );

            const int other_node = ( current_edge.first == current_node ) ? current_edge.second : current_edge.first;

            if( other_node == current_edge.first  ) assert( current_edge.second == current_node );
            if( other_node == current_edge.second ) assert( current_edge.first  == current_node );


            if( respect_capacity )
            {
                int x, y, z;
                std::tie(x,y,z) = graph.get_position_from_nodeindex( other_node );
                if( BB.minx > x or BB.maxx < x or BB.miny > y or BB.maxy < y or BB.minz > z or BB.maxz < z ) {
                    continue;
                }
            }

            
            
            const auto current_direction = graph.get_edge_direction( edgeindex );

            const int current_edge_capacity = graph.get_capacity( edgeindex );

            int required_capacity = 0;

            // if not in z direction, we need to check the capacity of the edge 
            if( current_direction != Graph::direction::z_plus ) 
            {

                const auto nodes = graph.get_nodes_of_edge( edgeindex );
                int x1, y1, z1, x2, y2, z2;
                std::tie( x1, y1, z1 ) = graph.get_position_from_nodeindex( nodes.first  );
                std::tie( x2, y2, z2 ) = graph.get_position_from_nodeindex( nodes.second );

                assert( z1 == z2 );
                assert( x1 == x2+1 or x1 == x2-1 or y1 == y2+1 or y1 == y2-1 );
                if( x1 != x2 ) assert( y1 == y2 );
                if( y1 != y2 ) assert( x1 == x2 );
                
                const auto min_spacing = problem.dimension.minimum_spacing[z1];
                const auto min_width   = problem.dimension.minimum_width[z1];

                required_capacity = min_spacing + std::max(min_width,min_net_width);
                
                assert( std::isfinite( required_capacity             ) );
                assert( std::isfinite( graph.get_capacity(edgeindex) ) );

                assert( std::isfinite( aggregated_width[edgeindex] ) );
                assert( 0 <= aggregated_width[edgeindex] );

                // If there is no capacity, throw out the edge 

                if( respect_capacity )
                if( aggregated_width[edgeindex] + required_capacity > current_edge_capacity ) continue;
            
            }

            // if( required_capacity + aggregated_width[edgeindex] > max_capacity ) std::clog << "Capacity ";
            
            if( respect_capacity ) {
                assert( required_capacity + aggregated_width[edgeindex] <= current_edge_capacity );
                assert( current_edge_capacity > 0 );
            }
            
            
            
            
            // calculate the costs of the edge // NOTE each direction has unit cost 
            int length_of_edge = 0;
            if( current_direction == Graph::direction::x_plus ) length_of_edge += 1; // problem.tileInfo.tile_width;
            if( current_direction == Graph::direction::y_plus ) length_of_edge += 1; // problem.tileInfo.tile_height;
            if( current_direction == Graph::direction::z_plus ) length_of_edge += 1;

            float edge_weight = length_of_edge; 
            
            if( not respect_capacity ) edge_weight += capacity_penalty_factor * std::max( 0.f, aggregated_width[edgeindex] - (float)current_edge_capacity );
            
            assert( std::isfinite( edge_weight ) );

            float new_distance = distance[current_node] + edge_weight;

            assert( queued[other_node] <= current_iteration );

            if( queued[other_node] < current_iteration ) {

                // if the other node has not been queued yet, then insert 

                assert( not pq.contains( other_node ) );

                pq.push( other_node, new_distance );

                queued[other_node]         = current_iteration;
                
                distance[other_node]       = new_distance;

                preceding_node[other_node] = current_node;

                relevant_edge[other_node]  = edgeindex;

            } else if( queued[other_node] == current_iteration && new_distance < distance[other_node] ) {

                // if the other node has been queued already, then consider updating the weight 

                // if the other node has a distance larger than what is possible from `current_node`,
                // then update or insert 
            
                assert( pq.contains( other_node ) );

                pq.setPriority( other_node, new_distance );
                
                distance[other_node]       = new_distance;

                preceding_node[other_node] = current_node;

                relevant_edge[other_node]  = edgeindex;

            } else {

                // std::clog << queued[other_node] <<' '<< current_iteration <<' '<< new_distance <<' '<< distance[other_node] <<'\n';
                // std::clog << distance[current_node] <<' '<< edge_weight <<'\n';
                assert( queued[other_node] == current_iteration );
                assert( std::isfinite( distance[other_node] ) );
                assert( std::isfinite( new_distance ) );
                assert( new_distance >= distance[other_node] );

            }

            

        } // iteration over edges 

        // we have processed all neighbors of the current node 

        if( active_T.contains(current_node) ) active_T.erase( current_node );

    } // while target non empty 

    // we have found all target nodes. Now collect all the edges 

    for( const auto t : T )
    {
        
        assert( queued[t] == current_iteration );

        int p = t;
        
        while( preceding_node[p] != -1 ){

            assert( queued[p] == current_iteration );

            int edgeindex = relevant_edge[p];

            assert( edgeindex != -1 );

            const auto edge = graph.get_nodes_of_edge(edgeindex);

            assert( edge.first == p or edge.second == p );

            ret.insert(edgeindex);

            p = preceding_node[p];

            assert( edge.first == p or edge.second == p );

        }

        assert( preceding_node[p] == -1 );
        assert( relevant_edge[p]  == -1 );

    }

    std::clog << "PQ capacity (finish): " << pq.capacity() << "\t max use " << max_pq_size << "\t iterations " << num_iterations << "\n";
    
    return ret;
}





#endif 
