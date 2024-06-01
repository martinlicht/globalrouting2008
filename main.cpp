/*
Copyright (c) 2024 Martin Werner Licht

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <cmath>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>

#include "common.hpp"
#include "connector.hpp"
#include "graph.hpp"
#include "grp.hpp"
#include "grp2graph.hpp"
#include "output_tree.hpp"

int main( int argc, char* argv[] )
{
    const std::string filename = ( argc > 1 ) ? argv[1] : "adaptec1.capo70.2d.35.50.90.gr";

    std::ifstream file( filename, std::ios_base::openmode::_S_in );

    if( !file ) {
        std::cerr << "Unable to open file: " << filename << "\n";
        return 1;
    } else {
        std::clog << "Opened file: " << filename << "\n";
    }

    GlobalRoutingProblem problem;
    problem.read( file );
    file.close();

    problem.heuristic_optimization();

    if( !problem.check() ) {
        std::cerr << "Data verification failed.\n";
        return 1;
    }

    std::clog << "Data verification succeeded.\n";

    // Convert to Graph

    std::clog << "Create Graph from problem data.\n";

    Graph graph = createGraphFromGlobalRoutingProblem( problem );

    std::clog << "Initialize routing class.\n";

    Connector connector = Connector( problem, graph );

    const auto trees = connector.connect();

    std::clog << "Routing complete. \n";

    // Write the data to an output file
    const std::string outputfilename = generate_new_filename( filename + ".solution" );
    std::ofstream     outfile( outputfilename, std::ios_base::openmode::_S_out );

    if( !outfile ) {
        std::cerr << "Unable to open output file\n";
        return 1;
    } else {
        std::clog << "Opened file: " << outputfilename << "\n";
    }

    for( int n = 0; n < problem.nets.size(); n++ ) {
        output_tree_for_net( outfile, problem, graph, n, trees[n] );
    }

    outfile.close();

    std::clog << "Finished. \n";

    return 0;
}
