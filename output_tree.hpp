/*
Copyright (c) 2024 Martin Werner Licht

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef IG_OUTPUT
#define IG_OUTPUT

#include <ostream>
#include <set>
#include <utility>

#include "common.hpp"
#include "graph.hpp"
#include "grp.hpp"

void output_tree_for_net( std::ostream& os, const GlobalRoutingProblem& grp, const Graph& graph, const int net_index, const std::set<int>& tree )
{
    assert( 0 <= net_index && net_index < grp.nets.size() );

    // Print the name and the id# of the net with index `net_index`
    const auto net = grp.nets[net_index];

    os << net.name << " " << net.id << " " << tree.size() << std::endl;

    // Print the respective edges

    for( const auto& edgeindex : tree ) {
        assert( 0 <= edgeindex && edgeindex < graph.count_edges() );

        const auto edge = graph.get_nodes_of_edge( edgeindex );

        int index_from = edge.first;
        int index_to   = edge.second;

        assert( 0 <= index_from && index_from < graph.count_nodes() );
        assert( 0 <= index_to && index_to < graph.count_nodes() );

        const auto tiles_from = graph.get_position_from_nodeindex( index_from );
        const auto tiles_to   = graph.get_position_from_nodeindex( index_to );

        int x_f, y_f, z_f;
        int x_t, y_t, z_t;

        std::tie( x_f, y_f ) = grp.center_of_tile( std::get<0>( tiles_from ), std::get<1>( tiles_from ) );
        std::tie( x_t, y_t ) = grp.center_of_tile( std::get<0>( tiles_to ), std::get<1>( tiles_to ) );

        z_f = std::get<2>( tiles_from );
        z_t = std::get<2>( tiles_to );

        assert( grp.tileInfo.lower_left_x <= x_f && x_f <= grp.tileInfo.lower_left_x + grp.grid.x_grids * grp.tileInfo.tile_width );
        assert( grp.tileInfo.lower_left_x <= x_t && x_t <= grp.tileInfo.lower_left_x + grp.grid.x_grids * grp.tileInfo.tile_width );

        assert( grp.tileInfo.lower_left_y <= y_f && y_f <= grp.tileInfo.lower_left_y + grp.grid.y_grids * grp.tileInfo.tile_height );
        assert( grp.tileInfo.lower_left_y <= y_t && y_t <= grp.tileInfo.lower_left_y + grp.grid.y_grids * grp.tileInfo.tile_height );

        assert( 0 <= z_f && z_f < grp.grid.layers );
        assert( 0 <= z_t && z_t < grp.grid.layers );

        // NOTE: layers are zero-based internally
        z_f++;
        z_t++;

        os << "(" << x_f << "," << y_f << "," << z_f << ")-(" << x_t << "," << y_t << "," << z_t << ")" << std::endl;
    }

    // Finish the tree with an exclamation mark

    os << "!" << std::endl << std::endl;

    // std::clog << "Completed file output" << std::endl << std::endl;

    return;
}

#endif
