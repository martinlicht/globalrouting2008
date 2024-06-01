/*
Copyright (c) 2024 Martin Werner Licht

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef IG_GRAPH
#define IG_GRAPH

#include <cassert>

#include <iostream>
#include <limits>
#include <tuple>
#include <utility>
#include <vector>

#include "common.hpp"

class Graph {

private:
    
    int dim_x;
    int dim_y;
    int dim_z;

    std::vector<int> capacities;
    // std::vector<int> min_widths;
    // std::vector<int> min_spacings;

    // for each edge, minimum width and min spacing, capacity 

public:
    
    static const int invalid_index;

    typedef std::pair<int, int> edge;

    enum class direction : signed int { x_plus, x_minus, y_plus, y_minus, z_plus, z_minus };

    static direction opposite_direction( direction dir )
    {
        switch( dir ) {
            case Graph::direction::x_plus:  return Graph::direction::x_minus;
            case Graph::direction::x_minus: return Graph::direction::x_plus;
            case Graph::direction::y_plus:  return Graph::direction::y_minus;
            case Graph::direction::y_minus: return Graph::direction::y_plus;
            case Graph::direction::z_plus:  return Graph::direction::z_minus;
            case Graph::direction::z_minus: return Graph::direction::z_plus;
        }
        assert(false);
    }

    Graph( int dim_x, int dim_y, int dim_z );

    int count_nodes() const;
    int count_edges() const;

    int get_nodeindex_from_position( int x, int y, int z ) const;
    std::tuple<int, int, int> get_position_from_nodeindex( int nodeindex ) const;
    int get_neighbor( int nodeindex, direction dir ) const;

    edge get_nodes_of_edge( int edge_index ) const;

    bool is_direction_possible( int nodeindex, direction dir ) const; 
    
    direction get_edge_direction( int edge_index ) const;

    std::vector<int> get_edgeindices_from_node( int nodeindex ) const;
    int get_edgeindex_from_node_and_direction( int nodeindex, direction dir ) const;
    int get_edgeindex_from_nodes( int nodeindex1, int nodeindex2 ) const;
    
    int get_capacity( int edgeindex ) const;
    void set_capacity( int edgeindex, int new_capacity );
    const std::vector<int>& get_capacities() const;

    // int get_weight( int edgeindex ) const;
    // void set_weight( int edgeindex, int new_weight );
};

const int Graph::invalid_index = -1;

std::ostream& operator<<( std::ostream& out, Graph::direction dir )
{
    switch( dir ) {
        case Graph::direction::x_plus:  out << "+x"; break; 
        case Graph::direction::x_minus: out << "-x"; break;
        case Graph::direction::y_plus:  out << "+y"; break;
        case Graph::direction::y_minus: out << "-y"; break;
        case Graph::direction::z_plus:  out << "+z"; break;
        case Graph::direction::z_minus: out << "-z"; break;
        default: assert(false);
    }
    return out;
}

Graph::Graph( int dim_x, int dim_y, int dim_z )
: 
dim_x(dim_x), 
dim_y(dim_y), 
dim_z(dim_z),
capacities(0)
// min_widths(0)
{
    assert( dim_x >= 1 && dim_y >= 1 && dim_z >= 1 );

    capacities.resize( 
        (dim_x  ) * (dim_y  ) * (dim_z-1)
        +
        (dim_x  ) * (dim_y-1) * (dim_z  )
        +
        (dim_x-1) * (dim_y  ) * (dim_z  )
    , 0 ); //, std::numeric_limits<float>::quiet_NaN() );

    // min_widths.resize( 
    //     (dim_x  ) * (dim_y  ) * (dim_z-1)
    //     +
    //     (dim_x  ) * (dim_y-1) * (dim_z  )
    //     +
    //     (dim_x-1) * (dim_y  ) * (dim_z  )
    // , 0 ); //, std::numeric_limits<float>::quiet_NaN() );

}

int Graph::count_nodes() const 
{ 
    return dim_x * dim_y * dim_z; 
}

int Graph::count_edges() const 
{ 
    int ret = (dim_x  ) * (dim_y  ) * (dim_z-1) 
              +
              (dim_x  ) * (dim_y-1) * (dim_z  ) 
              + 
              (dim_x-1) * (dim_y  ) * (dim_z  ); 
    assert( ret == capacities.size() );
    return ret;
}


int Graph::get_nodeindex_from_position( int x, int y, int z ) const 
{
    assert( 0 <= x && x < dim_x );
    assert( 0 <= y && y < dim_y );
    assert( 0 <= z && z < dim_z );
    
    return x * dim_y * dim_z + y * dim_z + z;
}

std::tuple<int, int, int> Graph::get_position_from_nodeindex( int nodeindex ) const 
{
    
    assert( nodeindex != invalid_index ); 
    assert( nodeindex >= 0 && nodeindex < dim_x * dim_y * dim_z );

    int n = nodeindex;

    int x = n / ( dim_y * dim_z );
    n %= ( dim_y * dim_z );

    assert( 0 <= x && x < dim_x );

    int y = n / dim_z;
    n %= dim_z;

    assert( 0 <= y && y < dim_y );

    int z = n;

    assert( 0 <= z && z < dim_z );

    assert( nodeindex == x * dim_y * dim_z + y * dim_z + z );

    return {x, y, z};
}

bool Graph::is_direction_possible( int nodeindex, direction dir ) const 
{
    
    assert( nodeindex != invalid_index && 0 <= nodeindex && nodeindex < dim_x * dim_y * dim_z ); 
    
    int x, y, z;
    std::tie(x, y, z) = get_position_from_nodeindex( nodeindex );
    
    assert( 0 <= x && x < dim_x );
    assert( 0 <= y && y < dim_y );
    assert( 0 <= z && z < dim_z );

    switch( dir ) {
        case Graph::direction::x_plus:  return x < dim_x-1;
        case Graph::direction::x_minus: return x >= 1;
        case Graph::direction::y_plus:  return y < dim_y-1;
        case Graph::direction::y_minus: return y >= 1;
        case Graph::direction::z_plus:  return z < dim_z-1;
        case Graph::direction::z_minus: return z >= 1;
        default: assert(false);
    }
    assert(false);
}

int Graph::get_neighbor( int nodeindex, direction dir ) const 
{
    assert( nodeindex != invalid_index && 0 <= nodeindex && nodeindex < dim_x * dim_y * dim_z ); 
    assert( is_direction_possible( nodeindex, dir ) );
    
    int x, y, z;
    std::tie( x, y, z ) = get_position_from_nodeindex( nodeindex );
    
    switch( dir ) {
        case Graph::direction::x_plus:  x++; break;
        case Graph::direction::x_minus: x--; break;
        case Graph::direction::y_plus:  y++; break;
        case Graph::direction::y_minus: y--; break;
        case Graph::direction::z_plus:  z++; break;
        case Graph::direction::z_minus: z--; break;
        default: assert(false);
    }
    
    // if( dir == Graph::direction::x_minus ) { x--; }
    // if( dir == Graph::direction::y_minus ) { y--; }
    // if( dir == Graph::direction::z_minus ) { z--; }
    // if( dir == Graph::direction::x_plus ) { x++; }
    // if( dir == Graph::direction::y_plus ) { y++; }
    // if( dir == Graph::direction::z_plus ) { z++; }

    return get_nodeindex_from_position( x, y, z);
    
}


int Graph::get_edgeindex_from_node_and_direction( int nodeindex, direction dir ) const 
{
    
    assert( nodeindex != invalid_index && 0 <= nodeindex && nodeindex < dim_x * dim_y * dim_z ); 
    assert( is_direction_possible( nodeindex, dir ) );

    int x, y, z;
    std::tie( x, y, z ) = get_position_from_nodeindex( nodeindex );

    assert( x >= 0 && y >= 0 && z >= 0 );
    assert( 0 <= x && x < dim_x );
    assert( 0 <= y && y < dim_y );
    assert( 0 <= z && z < dim_z );

    if( dir == Graph::direction::x_minus ) { x--; dir = Graph::direction::x_plus; }
    if( dir == Graph::direction::y_minus ) { y--; dir = Graph::direction::y_plus; }
    if( dir == Graph::direction::z_minus ) { z--; dir = Graph::direction::z_plus; }

    assert( not ( x == dim_x-1 && dir == Graph::direction::x_plus ) );
    assert( not ( y == dim_y-1 && dir == Graph::direction::y_plus ) );
    assert( not ( z == dim_z-1 && dir == Graph::direction::z_plus ) );
    
    if(        dir == Graph::direction::x_plus ) {
        
        int edgeindex = 0;
        assert( x < dim_x-1 && y < dim_y && z < dim_z );
        edgeindex += x * dim_y * dim_z + y * dim_z + z;
        assert( 0 <= edgeindex && edgeindex < count_edges() );
        return edgeindex;

    } else if( dir == Graph::direction::y_plus ) {
        
        int edgeindex = 0;
        assert( x < dim_x && y < dim_y-1 && z < dim_z );
        edgeindex += (dim_x-1) * dim_y * dim_z;
        edgeindex += y * dim_x * dim_z + x * dim_z + z;
        assert( 0 <= edgeindex && edgeindex < count_edges() );
        return edgeindex;

    } else if( dir == Graph::direction::z_plus ) {
        
        int edgeindex = 0;
        assert( x < dim_x && y < dim_y && z < dim_z-1 );
        edgeindex += (dim_x-1) * dim_y * dim_z;
        edgeindex += dim_x * (dim_y-1) * dim_z;
        edgeindex += z * dim_x * dim_y + x * dim_y + y;
        assert( 0 <= edgeindex && edgeindex < count_edges() );
        return edgeindex;

    } else {

        assert(false);

    }
    
    return invalid_index;
}


int Graph::get_edgeindex_from_nodes( int nodeindex1, int nodeindex2 ) const
{
    assert( nodeindex1 != invalid_index && 0 <= nodeindex1 && nodeindex1 < dim_x * dim_y * dim_z ); 
    assert( nodeindex2 != invalid_index && 0 <= nodeindex2 && nodeindex2 < dim_x * dim_y * dim_z ); 
    
    int x1, y1, z1;
    std::tie( x1, y1, z1 ) = get_position_from_nodeindex(nodeindex1);
    
    int x2, y2, z2;
    std::tie( x2, y2, z2 ) = get_position_from_nodeindex(nodeindex2);

    if( x1 == x2-1 && y1 == y2   && z1 == z2   ) return get_edgeindex_from_node_and_direction( nodeindex1, Graph::direction::x_plus );
    if( x1 == x2+1 && y1 == y2   && z1 == z2   ) return get_edgeindex_from_node_and_direction( nodeindex1, Graph::direction::x_minus );
    if( x1 == x2   && y1 == y2-1 && z1 == z2   ) return get_edgeindex_from_node_and_direction( nodeindex1, Graph::direction::y_plus );
    if( x1 == x2   && y1 == y2+1 && z1 == z2   ) return get_edgeindex_from_node_and_direction( nodeindex1, Graph::direction::y_minus );
    if( x1 == x2   && y1 == y2   && z1 == z2-1 ) return get_edgeindex_from_node_and_direction( nodeindex1, Graph::direction::z_plus );
    if( x1 == x2   && y1 == y2   && z1 == z2+1 ) return get_edgeindex_from_node_and_direction( nodeindex1, Graph::direction::z_minus );

    return invalid_index;
}

std::vector<int> Graph::get_edgeindices_from_node( int nodeindex ) const 
{
    
    assert( nodeindex != invalid_index && 0 <= nodeindex && nodeindex < dim_x * dim_y * dim_z ); 
    
    std::vector<int> edges;
    edges.reserve(6);
    
    for( int i = 0; i < 6; ++i ) {

        direction dir = static_cast<direction>(i);

        if( not is_direction_possible( nodeindex, dir ) ) continue;
        
        int edgeindex = get_edgeindex_from_node_and_direction( nodeindex, dir );
        assert( edgeindex != invalid_index && 0 <= edgeindex && edgeindex < count_edges() );
        for( const auto e : edges ) assert( e != edgeindex );
        edges.push_back( edgeindex );

    }
    
    edges.reserve( edges.size() );
    assert( edges.size() <= 6 );
    return edges;
}



Graph::direction Graph::get_edge_direction( int edge_index ) const 
{
    assert( 0 <= edge_index && edge_index < count_edges() );

    const auto edge = get_nodes_of_edge( edge_index );

    int x1, y1, z1, x2, y2, z2;
    std::tie( x1, y1, z1 ) = get_position_from_nodeindex( edge.first );
    std::tie( x2, y2, z2 ) = get_position_from_nodeindex( edge.second );
    
    if( x1 == x2-1 && y1 == y2   && z1 == z2   ) return Graph::direction::x_plus;
    if( x1 == x2+1 && y1 == y2   && z1 == z2   ) return Graph::direction::x_plus;
    if( x1 == x2   && y1 == y2-1 && z1 == z2   ) return Graph::direction::y_plus;
    if( x1 == x2   && y1 == y2+1 && z1 == z2   ) return Graph::direction::y_plus;
    if( x1 == x2   && y1 == y2   && z1 == z2-1 ) return Graph::direction::z_plus;
    if( x1 == x2   && y1 == y2   && z1 == z2+1 ) return Graph::direction::z_plus;

    assert(false);
}

Graph::edge Graph::get_nodes_of_edge( int edge_index ) const 
{ 
    
    assert( 0 <= edge_index && edge_index < count_edges() );

    // std::clog << edge_index << std::endl;

    int base_node;
    direction dir; 

    if( edge_index < (dim_x-1)*dim_y*dim_z ) {
        dir = Graph::direction::x_plus;
        int e = edge_index;
        
        int x = e / (dim_y*dim_z);
        e %= (dim_y*dim_z);
        int y = e / dim_z;
        e %= dim_z;
        int z = e;

        assert( 0 <= x && x < dim_x - 1 );
        assert( 0 <= y && y < dim_y     );
        assert( 0 <= z && z < dim_z     );

        assert( edge_index == x*dim_y*dim_z + y*dim_z + z );

        base_node = get_nodeindex_from_position( x, y, z );
    } else if( edge_index < (dim_x-1)*dim_y*dim_z + dim_x*(dim_y-1)*dim_z ) {
        
        assert( edge_index >= (dim_x-1)*dim_y*dim_z );

        dir = Graph::direction::y_plus;
        int e = edge_index - (dim_x-1)*dim_y*dim_z;
        
        int y = e / (dim_x*dim_z);
        e %= (dim_x*dim_z);
        int x = e / dim_z;
        e %= dim_z;
        int z = e;

        assert(0 <= x && x < dim_x);
        assert(0 <= y && y < dim_y - 1);
        assert(0 <= z && z < dim_z);

        assert( edge_index == (dim_x-1)*dim_y*dim_z + y*dim_x*dim_z + x*dim_z + z );
        
        base_node = get_nodeindex_from_position( x, y, z );
    } else {
        assert( edge_index >= (dim_x-1)*dim_y*dim_z + dim_x*(dim_y-1)*dim_z );
        
        assert( edge_index < (dim_x-1)*dim_y*dim_z + dim_x*(dim_y-1)*dim_z + dim_x*dim_y*(dim_z-1) );
        
        dir = Graph::direction::z_plus;
        int e = edge_index - (dim_x-1)*dim_y*dim_z - dim_x*(dim_y-1)*dim_z;
        
        int z = e / (dim_x*dim_y);
        e %= (dim_x*dim_y);
        int x = e / dim_y;
        e %= dim_y;
        int y = e;

        assert(0 <= x && x < dim_x);
        assert(0 <= y && y < dim_y);
        assert(0 <= z && z < dim_z - 1);

        assert( edge_index == (dim_x-1)*dim_y*dim_z + dim_x*(dim_y-1)*dim_z + z*dim_x*dim_y + x*dim_y + y );
        
        base_node = get_nodeindex_from_position( x, y, z );
    }
    
    int next_node = get_neighbor( base_node, dir );

    return { base_node, next_node };
}

int Graph::get_capacity( int edgeindex ) const 
{
    assert( edgeindex >= 0 && edgeindex < static_cast<int>(capacities.size()) );
    return capacities[edgeindex];
}

void Graph::set_capacity( int edgeindex, int new_capacity) 
{
    assert( edgeindex >= 0 && edgeindex < static_cast<int>(capacities.size()) );
    capacities[edgeindex] = new_capacity;
}

const std::vector<int>& Graph::get_capacities() const 
{
    return capacities;
}


// int Graph::get_weight( int edgeindex ) const {
//     assert( edgeindex >= 0 && edgeindex < static_cast<int>(min_widths.size()) );
//     return min_widths[edgeindex];
// }

// void Graph::set_weight( int edgeindex, int new_weight ) {
//     assert( edgeindex >= 0 && edgeindex < static_cast<int>(min_widths.size()) );
//     min_widths[edgeindex] = new_weight;
// }


#endif 
