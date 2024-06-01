/*
Copyright (c) 2024 Martin Werner Licht

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <iostream>
#include <vector>
#include <algorithm>
#include <cassert>

#include "common.hpp"

#include "priority_queue.hpp"

using namespace std;


// Unit test function
void testPriorityQueue() {
    
    typedef typename PriorityQueue<>::Entry Entry;
    typedef typename PriorityQueue<>::prioritytype prioritytype;

    for( int N = 0; N <= 20; N++ )
    // for( int T = 0; T <  10; T++ )
    {
        clog << "Testing with " << N << " entries\n";

        PriorityQueue pq;

        // Create a vector of entries with randomized priorities
        vector<Entry> entries;
        for( int i = 0; i < N; ++i) {
            Entry entry = {i, 0.1f * prioritytype( rand() % 100 ) }; // Random priority
            // clog << entry.value << ' ' << entry.priority << endl;
            entries.push_back(entry);
        }

        // Sort the entries by priority
        sort(entries.begin(), entries.end(), [](const Entry& a, const Entry& b) {
            return a.priority < b.priority || ( a.priority == b.priority && a.value < b.value );
        });

        // Create a shuffled copy of entries
        vector<Entry> shuffled_entries = entries;
        random_shuffle(shuffled_entries.begin(), shuffled_entries.end());

        // Insert shuffled entries into the priority queue
        for( const auto& entry : shuffled_entries) {
            pq.push(entry.value, entry.priority);
        }

        // printEntryArray<>( entries );
        // printEntryArray<>( shuffled_entries );
        
        // Check that the entries come out in the correct order
        for( const auto& entry : entries) {
            Entry popped = pq.pop();
            
            // clog << entry.value << ' ' << entry.priority << endl;
            // clog << popped.value << ' ' << popped.priority << endl;

            assert(popped.value == entry.value);
            assert(popped.priority == entry.priority);
        }

        assert( pq.empty() );
    }

    for( int N = 0; N <= 20; N++ )
    // for( int T = 0; T <  10; T++ )
    {
        clog << "Advanced testing with " << N << " entries\n";

        if( N == 0 ) continue;

        PriorityQueue pq;

        // Create a vector of entries with randomized priorities
        vector<Entry> entries;
        for( int i = 0; i < N; ++i) {
            Entry entry = {i, 0.1f * prioritytype( rand() % 100 ) }; // Random priority
            entries.push_back(entry);
        }

        random_shuffle( entries.begin(), entries.end());

        // Insert entries into the priority queue
        for( const auto& entry : entries) {
            pq.push(entry.value, entry.priority);
        }

        printEntryArray<decltype(pq)::valuetype,decltype(pq)::prioritytype>( entries );
        pq.print();

        // randomly reassign priorities 
        for( int t = 0; t < 40; t++ )
        {
            int i = rand() % entries.size();

            prioritytype w = 0.1f * prioritytype( rand() % 100 );
            
            entries[i].priority = w;
            pq.setPriority( entries[i].value, w );
        }
        
        // Once more, sort the entries by priority
        sort(entries.begin(), entries.end(), [](const Entry& a, const Entry& b) {
            return a.priority < b.priority || ( a.priority == b.priority && a.value < b.value );
        });

        printEntryArray<decltype(pq)::valuetype,decltype(pq)::prioritytype>( entries );
        pq.print();

        // Check that the entries come out in the correct order
        for( const auto& entry : entries ) {
            Entry popped = pq.pop();
            
            clog << entry.value << ' ' << entry.priority << endl;
            clog << popped.value << ' ' << popped.priority << endl;

            assert(popped.value == entry.value);
            assert(popped.priority == entry.priority);
        }

        assert( pq.empty() );
    }

    clog << "Priority queue unit test passed!" << endl;
}


int main() {
    
    // Run the unit test
    testPriorityQueue();

    return 0;
}