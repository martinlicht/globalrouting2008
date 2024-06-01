/*
Copyright (c) 2024 Martin Werner Licht

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <map>
#include <tuple>
#include <cmath>

#include "common.hpp"

#include "grp.hpp"

int main() {
    
    const std::string filename = "adaptec1.capo70.2d.35.50.90.gr";
    std::ifstream file( filename, std::ios_base::openmode::_S_in );
    if( !file) {
        std::cerr << "Unable to open file: " << filename << "\n";
        return 1;
    }
    
    GlobalRoutingProblem problem;
    problem.read(file);
    file.close();
    
    if( !problem.check()) {
        std::cerr << "Data verification failed.\n";
        return 1;
    }

    std::clog << "Data verification succeeded.\n";

    // Example of using stream operators
    std::clog << problem.grid << std::endl;
    std::clog << problem.capacity << std::endl;
    std::clog << problem.dimension << std::endl;
    std::clog << problem.tileInfo << std::endl;

    for( const auto &net : problem.nets) {
        std::clog << net << std::endl;
    }

    for( const auto &capAdj : problem.capacityAdjustments) {
        std::clog << capAdj << std::endl;
    }


    // Write the data to an output file
    std::ofstream outfile( generate_new_filename( filename + ".clone" ), std::ios_base::openmode::_S_out );
    if( !outfile) {
        std::cerr << "Unable to open output file\n";
        return 1;
    }

    problem.write( outfile );
    outfile.close();
    
    return 0;
}


