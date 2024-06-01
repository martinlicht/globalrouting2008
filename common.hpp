#ifndef IG_COMMON
#define IG_COMMON

#include <cassert>
#include <fstream>

const char nl = '\n';

const char tab = '\t';

const char space = ' ';

bool file_exists( const std::string& name )
{
    std::ifstream file( name.c_str() );
    return file.good();
}

std::string generate_new_filename( const std::string& original_filename )
{
    if( not file_exists( original_filename ) ) {
        return original_filename;  // File does not exist, return the original name
    }

    std::string new_filename = "";

    int num = 0;

    do {
        new_filename = original_filename + "." + std::to_string( num );
        // std::clog << "Test: " << new_filename << nl;
        num++;
    } while( file_exists( new_filename ) );

    assert( not file_exists( new_filename ) );

    return new_filename;
}

#endif
