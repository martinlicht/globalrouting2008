/*
Copyright (c) 2024 Martin Werner Licht

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <cassert>

#include <iostream>
#include <tuple>
#include <utility>
#include <vector>

#include "common.hpp"

#include "graph.hpp"

int main() {
    
    const std::vector<Graph::direction> list_of_directions = { 
        Graph::direction::x_plus, Graph::direction::x_minus, 
        Graph::direction::y_plus, Graph::direction::y_minus, 
        Graph::direction::z_plus, Graph::direction::z_minus
    };
            
    for( int Nx = 1; Nx <= 3; Nx++ )
    for( int Ny = 1; Ny <= 4; Ny++ )
    for( int Nz = 1; Nz <= 5; Nz++ )
    {
        Graph graph( Nx, Ny, Nz );
        
        std::clog << "Nx: " << Nx << " Ny: " << Ny << " Nz: " << Nz << std::endl;

        for( int nx = 0; nx < Nx; nx++ )
        for( int ny = 0; ny < Ny; ny++ )
        for( int nz = 0; nz < Nz; nz++ )
        {
            int nodeindex = graph.get_nodeindex_from_position(nx, ny, nz);

            assert( nodeindex != Graph::invalid_index );
            
            // std::clog << "Node index for position " << nx << ' ' << ny << ' ' << nz << " : " << nodeindex << std::endl;
            
            int x, y, z;
            std::tie( x, y, z ) = graph.get_position_from_nodeindex(nodeindex);

            assert( x == nx && y == ny && z == nz );

            for( const auto dir : list_of_directions )
            {

                if( x == 0 && dir == Graph::direction::x_minus ) continue;
                if( y == 0 && dir == Graph::direction::y_minus ) continue;
                if( z == 0 && dir == Graph::direction::z_minus ) continue;
                
                if( x == Nx-1 && dir == Graph::direction::x_plus ) continue;
                if( y == Ny-1 && dir == Graph::direction::y_plus ) continue;
                if( z == Nz-1 && dir == Graph::direction::z_plus ) continue;
                
                // std::clog << "Do " << x << ' ' << y << ' ' << z << ' ' << dir << " : " << nodeindex << std::endl;
            
                assert( graph.is_direction_possible( nodeindex, dir ) );
                int neighbor = graph.get_neighbor( nodeindex, dir );

                int x2, y2, z2;
                std::tie( x2, y2, z2 ) = graph.get_position_from_nodeindex(neighbor);
                // std::clog << "Neighbor " << x2 << ' ' << y2 << ' ' << z2 << " : " << neighbor << std::endl;

                assert( graph.get_nodeindex_from_position( x2, y2, z2) == neighbor );

                if( dir == Graph::direction::x_minus ) assert( x-1==x2 );
                if( dir == Graph::direction::y_minus ) assert( y-1==y2 );
                if( dir == Graph::direction::z_minus ) assert( z-1==z2 );
                
                if( dir == Graph::direction::x_plus ) assert( x+1==x2 );
                if( dir == Graph::direction::y_plus ) assert( y+1==y2 );
                if( dir == Graph::direction::z_plus ) assert( z+1==z2 );

                const auto opp = Graph::opposite_direction( dir );
                
                // std::clog << dir << ' ' << opp << std::endl;
                
                assert( graph.is_direction_possible( neighbor, opp ) );
                assert( graph.get_neighbor( neighbor, opp ) == nodeindex );
            
            }
            
            auto edges = graph.get_edgeindices_from_node(nodeindex);
            std::clog << "Edges for node index " << nodeindex << ": ";
            
            for( int edgeindex : edges) {
            
                std::clog << edgeindex << " ";
                
                int node1, node2;
                std::tie( node1, node2 ) = graph.get_nodes_of_edge( edgeindex );
                
                std::clog << "(" << node1 << "," << node2 << ") ";
                
                assert( node1 == nodeindex || node2 == nodeindex );
            
            }
            
            std::clog << std::endl;

            for( int e = 0; e < graph.count_edges(); e++ )
            {
                graph.set_capacity(0, 10.0 );
                // graph.set_weight(0, 20.0 );
                // std::clog << "Capacity for edge 0: " << graph.get_capacity(0) << std::endl;
            }
            
        }
        
    }
    
    return 0;
}
