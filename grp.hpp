/*
Copyright (c) 2024 Martin Werner Licht

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef IG_GRP
#define IG_GRP

#include <algorithm>
#include <cassert>
#include <cmath>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>

#include "common.hpp"

struct Grid {
    int x_grids;
    int y_grids;
    int layers;
};

struct Capacity {
    std::vector<int> vertical;
    std::vector<int> horizontal;
};

struct Dimension {
    std::vector<int> minimum_width;
    std::vector<int> minimum_spacing;
    std::vector<int> via_spacing;
};

struct TileInfo {
    int lower_left_x;
    int lower_left_y;
    int tile_width;
    int tile_height;
};

struct Pin {
    int x;
    int y;
    int layer;
};

struct Net {
    std::string      name;
    int              id;
    int              num_pins;
    int              minimum_width;
    std::vector<Pin> pins;
};

struct CapacityAdjustment {
    int col_start;
    int row_start;
    int layer_start;
    int col_end;
    int row_end;
    int layer_end;
    int adjusted_capacity;
};

// Stream operators for output
std::ostream &operator<<( std::ostream &os, const Grid &grid )
{
    os << "Grid: " << grid.x_grids << " x " << grid.y_grids << ", Layers: " << grid.layers;
    return os;
}

std::ostream &operator<<( std::ostream &os, const Capacity &capacity )
{
    os << "Vertical Capacity: ";
    for( const auto &v : capacity.vertical ) {
        os << v << " ";
    }
    os << "\nHorizontal Capacity: ";
    for( const auto &h : capacity.horizontal ) {
        os << h << " ";
    }
    return os;
}

std::ostream &operator<<( std::ostream &os, const Dimension &dimension )
{
    os << "Minimum Width: ";
    for( const auto &w : dimension.minimum_width ) {
        os << w << " ";
    }
    os << "\nMinimum Spacing: ";
    for( const auto &s : dimension.minimum_spacing ) {
        os << s << " ";
    }
    os << "\nVia Spacing: ";
    for( const auto &v : dimension.via_spacing ) {
        os << v << " ";
    }
    return os;
}

std::ostream &operator<<( std::ostream &os, const TileInfo &tileInfo )
{
    os << "Lower Left: (" << tileInfo.lower_left_x << ", " << tileInfo.lower_left_y << "),"
       << "Tile Width: " << tileInfo.tile_width << ", Tile Height: " << tileInfo.tile_height;
    return os;
}

std::ostream &operator<<( std::ostream &os, const Pin &pin )
{
    os << "Pin: (" << pin.x << ", " << pin.y << "), Layer: " << pin.layer;
    return os;
}

std::ostream &operator<<( std::ostream &os, const Net &net )
{
    os << "Net: " << net.name                       //
       << ", ID: " << net.id                        //
       << ", Number of Pins: " << net.num_pins      //
       << ", Minimum Width: " << net.minimum_width  //
       << "\nPins:\n";
    for( const auto &pin : net.pins ) {
        os << pin << "\n";
    }
    return os;
}

std::ostream &operator<<( std::ostream &os, const CapacityAdjustment &capAdj )
{
    os << "Capacity Adjustment: (" << capAdj.col_start << ", " << capAdj.row_start << ", " << capAdj.layer_start + 1  // NOTE: layers are zero-based internally
       << ") -> (" << capAdj.col_end << ", " << capAdj.row_end << ", " << capAdj.layer_end + 1                        // NOTE: layers are zero-based internally
       << "), Reduced Capacity: " << capAdj.adjusted_capacity;
    return os;
}

class GlobalRoutingProblem
{
  public:

    Grid                            grid;
    Capacity                        capacity;
    Dimension                       dimension;
    TileInfo                        tileInfo;
    std::vector<Net>                nets;
    std::vector<CapacityAdjustment> capacityAdjustments;

    void read( std::istream &is );

    bool check() const;

    void write( std::ostream &os ) const;

    void heuristic_optimization();

    std::pair<int, int> tile_of_coordinate( int x, int y ) const;

    std::pair<int, int> center_of_tile( int r, int c ) const;
};

void GlobalRoutingProblem::read( std::istream &is )
{
    std::string word;

    // Read grid
    is >> word >> grid.x_grids >> grid.y_grids >> grid.layers;

    // vertical capacities
    is >> word >> word;
    capacity.vertical.reserve( grid.layers );
    for( int i = 0; i < grid.layers; ++i ) {
        int cap;
        is >> cap;
        capacity.vertical.push_back( cap );
    }

    // horizontal capacities
    is >> word >> word;
    capacity.horizontal.reserve( grid.layers );
    for( int i = 0; i < grid.layers; ++i ) {
        int cap;
        is >> cap;
        capacity.horizontal.push_back( cap );
    }

    // min widths
    is >> word >> word;
    dimension.minimum_width.reserve( grid.layers );
    for( int i = 0; i < grid.layers; ++i ) {
        int width;
        is >> width;
        dimension.minimum_width.push_back( width );
    }

    // min spacing
    is >> word >> word;
    dimension.minimum_spacing.reserve( grid.layers );
    for( int i = 0; i < grid.layers; ++i ) {
        int spacing;
        is >> spacing;
        dimension.minimum_spacing.push_back( spacing );
    }

    // via spacing
    is >> word >> word;
    dimension.via_spacing.reserve( grid.layers );
    for( int i = 0; i < grid.layers; ++i ) {
        int spacing;
        is >> spacing;
        dimension.via_spacing.push_back( spacing );
    }

    // Read tile info
    is >> tileInfo.lower_left_x >> tileInfo.lower_left_y >> tileInfo.tile_width >> tileInfo.tile_height;

    // Read nets
    int num_nets;
    is >> word >> word >> num_nets;

    nets.reserve( num_nets );
    for( int i = 0; i < num_nets; ++i ) {
        Net net;
        is >> net.name >> net.id >> net.num_pins >> net.minimum_width;

        net.pins.reserve( net.num_pins );
        for( int j = 0; j < net.num_pins; ++j ) {
            Pin pin;
            is >> pin.x >> pin.y >> pin.layer;

            // The layer number is one-based. We make it zero-based:
            pin.layer--;

            net.pins.push_back( pin );
        }

        nets.push_back( net );
    }

    // Read capacity adjustments
    int num_capacity_adjustments;

    is >> num_capacity_adjustments;

    capacityAdjustments.reserve( num_capacity_adjustments );

    for( int i = 0; i < num_capacity_adjustments; ++i ) {
        CapacityAdjustment capAdj;
        is >> capAdj.col_start >> capAdj.row_start >> capAdj.layer_start >> capAdj.col_end >> capAdj.row_end >> capAdj.layer_end >> capAdj.adjusted_capacity;

        // NOTE: layers are zero-based internally
        capAdj.layer_start--;
        capAdj.layer_end--;

        capacityAdjustments.push_back( capAdj );
    }
}

bool GlobalRoutingProblem::check() const
{
    const auto grid      = this->grid;
    const auto capacity  = this->capacity;
    const auto dimension = this->dimension;
    const auto tileInfo  = this->tileInfo;
    const auto nets      = this->nets;

    // Check grid dimensions
    if( grid.x_grids <= 0 || grid.y_grids <= 0 || grid.layers <= 0 ) {
        std::cerr << "Invalid grid dimensions.\n";
        return false;
    }

    // Check capacities
    if( capacity.vertical.size() != grid.layers || capacity.horizontal.size() != grid.layers ) {
        std::cerr << "Capacity size does not match number of layers.\n";
        return false;
    }

    // Check dimensions
    if( dimension.minimum_width.size() != grid.layers || dimension.minimum_spacing.size() != grid.layers || dimension.via_spacing.size() != grid.layers ) {
        std::cerr << "Dimension size does not match number of layers.\n";
        return false;
    }

    // Check tile info
    if( tileInfo.tile_width <= 0 || tileInfo.tile_height <= 0 ) {
        std::cerr << "Invalid tile dimensions.\n";
        return false;
    }

    for( const auto c : capacity.vertical ) assert( c >= 0 );
    for( const auto c : capacity.horizontal ) assert( c >= 0 );
    for( const auto mw : dimension.minimum_width ) assert( mw >= 0 );
    for( const auto ms : dimension.minimum_spacing ) assert( ms >= 0 );
    for( const auto vs : dimension.via_spacing ) assert( vs >= 0 );

    // Check nets and pins
    for( const auto &net : nets ) {
        if( net.num_pins != static_cast<int>( net.pins.size() ) ) {
            std::cerr << "Number of pins for net " << net.name << " does not match the specified number.\n";
            return false;
        }

        for( const auto &pin : net.pins ) {
            // if( pin.layer <= 0 || pin.layer > grid.layers) { // NOTE: we save the layer as zero-based index
            if( pin.layer < 0 || pin.layer >= grid.layers ) {
                std::cerr << "Invalid layer for pin in net " << net.name << ".\n";
                return false;
            }
            if( pin.x < tileInfo.lower_left_x || pin.x > ( tileInfo.lower_left_x + grid.x_grids * tileInfo.tile_width ) || pin.y < tileInfo.lower_left_y || pin.y > ( tileInfo.lower_left_y + grid.y_grids * tileInfo.tile_height ) ) {
                std::cerr << "Pin coordinates out of bounds for net " << net.name << ".\n";
                return false;
            }
        }
    }

    // Check capacity adjustments
    for( const auto &capAdj : capacityAdjustments ) {
        if( capAdj.col_start < 0 || capAdj.col_start >= grid.x_grids || capAdj.row_start < 0 || capAdj.row_start >= grid.y_grids ||
            // capAdj.layer_start <= 0 || capAdj.layer_start > grid.layers // NOTE: layers are zero-based internally
            capAdj.layer_start < 0 || capAdj.layer_start >= grid.layers || capAdj.col_end < 0 || capAdj.col_end >= grid.x_grids || capAdj.row_end < 0 || capAdj.row_end >= grid.y_grids ||
            // capAdj.layer_end <= 0 || capAdj.layer_end > grid.layers // NOTE: layers are zero-based internally
            capAdj.layer_end < 0 || capAdj.layer_end >= grid.layers ) {
            std::cerr << "Invalid capacity adjustment coordinates.\n";
            return false;
        }
    }

    // If all checks pass
    return true;
}

void GlobalRoutingProblem::write( std::ostream &os ) const
{
    const auto &problem = *this;

    // Write grid information
    os << "grid\t" << problem.grid.x_grids << " " << problem.grid.y_grids << " " << problem.grid.layers << "\n";

    // Write vertical capacity
    os << "vertical capacity\t";
    for( const auto &cap : problem.capacity.vertical ) {
        os << " " << cap;
    }
    os << "\n";

    // Write horizontal capacity
    os << "horizontal capacity\t";
    for( const auto &cap : problem.capacity.horizontal ) {
        os << " " << cap;
    }
    os << "\n";

    // Write minimum width
    os << "minimum width\t";
    for( const auto &width : problem.dimension.minimum_width ) {
        os << " " << width;
    }
    os << "\n";

    // Write minimum spacing
    os << "minimum spacing\t";
    for( const auto &spacing : problem.dimension.minimum_spacing ) {
        os << " " << spacing;
    }
    os << "\n";

    // Write via spacing
    os << "via spacing\t";
    for( const auto &spacing : problem.dimension.via_spacing ) {
        os << " " << spacing;
    }
    os << "\n";

    // Write tile info
    os << problem.tileInfo.lower_left_x << " " << problem.tileInfo.lower_left_y << " " << problem.tileInfo.tile_width << " " << problem.tileInfo.tile_height << "\n";

    os << "\n";

    // Write nets
    os << "num net " << problem.nets.size() << "\n";
    for( const auto &net : problem.nets ) {
        os << net.name << " " << net.id << " " << net.num_pins << " " << net.minimum_width << "\n";
        for( const auto &pin : net.pins ) {
            os << pin.x << " " << pin.y << " " << pin.layer + 1  // NOTE: layers are zero-based internally
               << "\n";
        }
    }

    os << "\n";

    // Write capacity adjustments
    os << problem.capacityAdjustments.size() << "\n";
    for( const auto &capAdj : problem.capacityAdjustments ) {
        os << capAdj.col_start << " " << capAdj.row_start << " " << capAdj.layer_start + 1    // NOTE: layers are zero-based internally
           << "\t" << capAdj.col_end << " " << capAdj.row_end << " " << capAdj.layer_end + 1  // NOTE: layers are zero-based internally
           << "\t" << capAdj.adjusted_capacity << "\n";
    }
}

void GlobalRoutingProblem::heuristic_optimization()
{
    float average_number_of_pins = 0.0;

    for( unsigned int u = 0; u < nets.size(); u++ ) {
        Net &net = nets[u];

        std::vector<Pin> &pins = net.pins;

        average_number_of_pins += pins.size();

        float mx = 0.0;
        float my = 0.0;
        float mz = 0.0;

        for( auto it = pins.begin(); it != pins.end(); it++ ) {
            mx += it->x;
            my += it->y;
            mz += it->layer;
        }

        mx /= pins.size();
        my /= pins.size();
        mz /= pins.size();

        /* Sort pins according to distance to midpoint of the net */
        for( unsigned int m = 0; m < pins.size(); m++ )
            for( unsigned int n = 0; n < pins.size(); n++ ) {
                float diff1 = fabs( pins[m].x - mx ) + fabs( pins[m].y - my ) + fabs( pins[m].layer - mz );
                float diff2 = fabs( pins[n].x - mx ) + fabs( pins[n].y - my ) + fabs( pins[n].layer - mz );

                if( diff1 > diff2 ) std::swap( pins[m], pins[n] );
            }
    };

    average_number_of_pins /= nets.size();

    std::clog << "Try to optimize order of nets" << std::endl;

    std::sort( nets.begin(), nets.end(), []( auto &a, auto &b ) { return a.pins.size() < b.pins.size(); } );

    std::clog << "Average net size: " << average_number_of_pins << std::endl;
    std::clog << "Optimization done." << std::endl;
}

std::pair<int, int> GlobalRoutingProblem::tile_of_coordinate( int x, int y ) const
{
    int tx = std::floor( ( x - this->tileInfo.lower_left_x ) / (float)this->tileInfo.tile_width );
    int ty = std::floor( ( y - this->tileInfo.lower_left_y ) / (float)this->tileInfo.tile_height );
    assert( 0 <= tx && tx < this->grid.x_grids );
    assert( 0 <= ty && ty < this->grid.y_grids );
    return { tx, ty };
};

std::pair<int, int> GlobalRoutingProblem::center_of_tile( int tx, int ty ) const
{
    assert( 0 <= tx && tx < this->grid.x_grids );
    assert( 0 <= ty && ty < this->grid.y_grids );
    int x = this->tileInfo.lower_left_x + ( tx ) * this->tileInfo.tile_width + this->tileInfo.tile_width / 2;
    int y = this->tileInfo.lower_left_y + ( ty ) * this->tileInfo.tile_height + this->tileInfo.tile_height / 2;
    return { x, y };
};

#endif