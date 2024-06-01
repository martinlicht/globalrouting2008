/*
Copyright (c) 2024 Martin Werner Licht

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef IG_GRP2GRAPH
#define IG_GRP2GRAPH

#include "common.hpp"

#include "graph.hpp"
#include "grp.hpp"

Graph createGraphFromGlobalRoutingProblem( const GlobalRoutingProblem &problem )
{
    
    Graph graph( problem.grid.x_grids, problem.grid.y_grids, problem.grid.layers );

    // Initialize the capacities
    for( int x = 0; x < problem.grid.x_grids; ++x ) 
    for( int y = 0; y < problem.grid.y_grids; ++y ) 
    for( int z = 0; z < problem.grid.layers;  ++z )
    {
        int nodeindex = graph.get_nodeindex_from_position(x, y, z);

        if( graph.is_direction_possible( nodeindex, Graph::direction::x_plus ) ) {
            int edgeindex = graph.get_edgeindex_from_node_and_direction( nodeindex, Graph::direction::x_plus );
            graph.set_capacity( edgeindex, problem.capacity.horizontal[z] );
            // graph.set_weight(edgeindex, 1. );
        }
        if( graph.is_direction_possible( nodeindex, Graph::direction::x_minus ) ) {
            int edgeindex = graph.get_edgeindex_from_node_and_direction( nodeindex, Graph::direction::x_minus );
            graph.set_capacity( edgeindex, problem.capacity.horizontal[z] );
            // graph.set_weight(edgeindex, 1. );
        }
        if( graph.is_direction_possible( nodeindex, Graph::direction::y_plus ) ) {
            int edgeindex = graph.get_edgeindex_from_node_and_direction( nodeindex, Graph::direction::y_plus );
            graph.set_capacity( edgeindex, problem.capacity.vertical[z] );
            // graph.set_weight(edgeindex, 1. );
        }
        if( graph.is_direction_possible( nodeindex, Graph::direction::y_minus ) ) {
            int edgeindex = graph.get_edgeindex_from_node_and_direction( nodeindex, Graph::direction::y_minus );
            graph.set_capacity( edgeindex, problem.capacity.vertical[z] );
            // graph.set_weight(edgeindex, 1. );
        }
        if( graph.is_direction_possible( nodeindex, Graph::direction::z_plus ) ) {
            int edgeindex = graph.get_edgeindex_from_node_and_direction( nodeindex, Graph::direction::z_plus );
            graph.set_capacity( edgeindex, std::numeric_limits< int >::max() );  // Default capacity for z-direction
            // graph.set_weight(edgeindex, 1. );
        }
        if( graph.is_direction_possible( nodeindex, Graph::direction::z_minus ) ) {
            int edgeindex = graph.get_edgeindex_from_node_and_direction( nodeindex, Graph::direction::z_minus );
            graph.set_capacity( edgeindex, std::numeric_limits< int >::max() );  // Default capacity for z-direction
            // graph.set_weight(edgeindex, 1. );
        }
    }

    // Apply capacity adjustments
    for( const auto &capAdj : problem.capacityAdjustments ) 
    {
        
        int start_nodeindex = graph.get_nodeindex_from_position( capAdj.col_start, capAdj.row_start, capAdj.layer_start ); // NOTE: layers are zero-based internally 
        
        int end_nodeindex   = graph.get_nodeindex_from_position( capAdj.col_end,   capAdj.row_end,   capAdj.layer_end   ); // NOTE: layers are zero-based internally 
        
        assert( start_nodeindex != Graph::invalid_index && 0 <= start_nodeindex && start_nodeindex < graph.count_nodes() );
        assert( end_nodeindex   != Graph::invalid_index && 0 <=   end_nodeindex &&   end_nodeindex < graph.count_nodes() );

        int edgeindex = graph.get_edgeindex_from_nodes( start_nodeindex, end_nodeindex );

        assert( edgeindex != Graph::invalid_index && 0 <= edgeindex && edgeindex < graph.count_edges() );

        assert( capAdj.adjusted_capacity <= graph.get_capacity( edgeindex ) );
        
        graph.set_capacity( edgeindex, capAdj.adjusted_capacity );

        {
            int x1, y1, z1, x2, y2, z2;
            std::tie(x1,y1,z1) = graph.get_position_from_nodeindex( start_nodeindex );
            std::tie(x2,y2,z2) = graph.get_position_from_nodeindex(   end_nodeindex );
            assert( capAdj.col_start   == x1 && capAdj.col_end   == x2 );
            assert( capAdj.row_start   == y1 && capAdj.row_end   == y2 );
            assert( capAdj.layer_start == z1 && capAdj.layer_end == z2 );
        }
        
    }

    for( int e = 0; e < graph.count_edges(); e++ )
    {
        auto cap = graph.get_capacity( e );
        assert( std::isfinite( cap ) );
        assert( cap >= 0 );
    }

    return graph;
}


#endif
