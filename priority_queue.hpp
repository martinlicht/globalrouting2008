/*
Copyright (c) 2024 Martin Werner Licht

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef IG_PQ
#define IG_PQ

#include <algorithm>
#include <cassert>
#include <iostream>
#include <vector>

#include "common.hpp"

using namespace std;

template<typename Valuetype = long, typename Prioritytype = float>
class PriorityQueue
{
  public:

    typedef Valuetype    valuetype;
    typedef Prioritytype prioritytype;

    struct Entry {
        valuetype    value;
        prioritytype priority;

        bool operator<( const Entry& b ) const
        {
            const auto a = *this;
            return a.priority < b.priority || ( a.priority == b.priority && a.value < b.value );
        }
    };

  private:

    vector<Entry> heap;

    // Helper function for maintaining heap properties:
    // move item up as much as possible
    int heapifyUp( int index )
    {
        assert( 0 <= index && index < heap.size() );

        while( index > 0 ) {
            int parent = ( index - 1 ) / 2;

            if( heap[index] < heap[parent] ) {
                swap( heap[index], heap[parent] );
                index = parent;
            } else {
                break;
            }
        }

        return index;
    }

    // Helper function for maintaining heap properties:
    // move item down as much as possible
    int heapifyDown( int index )
    {
        assert( 0 <= index && index < heap.size() );

        const auto size = heap.size();

        while( true ) {
            int leftChild  = 2 * index + 1;
            int rightChild = 2 * index + 2;
            int smallest   = index;

            if( leftChild < size && heap[leftChild] < heap[smallest] ) {
                smallest = leftChild;
            }
            if( rightChild < size && heap[rightChild] < heap[smallest] ) {
                smallest = rightChild;
            }

            if( smallest != index ) {
                swap( heap[index], heap[smallest] );
                index = smallest;
            } else {
                break;
            }
        }

        return index;
    }

  public:

    // Constructor
    PriorityQueue() {}

    // print
    void print() const
    {
        for( const auto& entry : heap ) {
            clog << entry.value << ':' << entry.priority << "\t";
        }
        clog << endl;
    }

    int size() const { return heap.size(); }

    int capacity() const { return heap.capacity(); }

    // Check if the priority queue is empty
    bool empty() const
    {
        assert( heap.empty() == ( heap.size() == 0 ) );
        return heap.empty();
    }

    // Clear the queue completey
    void clear()
    {
        heap.clear();
        assert( heap.size() == 0 );
    }

    // Check whether any entry has a given value
    bool contains( Valuetype value ) const
    {
        return any_of( heap.begin(), heap.end(), [value]( const Entry& entry ) { return entry.value == value; } );
    }

    // peek the top entry
    Entry peek()
    {
        assert( not heap.empty() );

        if( heap.empty() ) {
            cerr << "Priority queue is empty!" << endl;
            exit( 1 );
        }

        return heap[0];
    }

    // Get the priority of any given value
    Prioritytype getPriority( Valuetype value ) const
    {
        auto it = find_if( heap.begin(), heap.end(), [value]( const Entry& entry ) { return entry.value == value; } );

        assert( it != heap.end() );

        if( it != heap.end() ) {
            return it->priority;
        } else {
            cerr << "Value not found in priority queue!" << endl;
            exit( 1 );
        }
    }

    // Insert an entry into the priority queue
    // NOTE: it is inserted at the end, so we need to move it up
    void push( valuetype value, prioritytype priority )
    {
        Entry entry = { value, priority };
        heap.push_back( entry );
        heapifyUp( heap.size() - 1 );
    }

    // Remove and return the entry with the highest priority
    // NOTE: We put the last member at the front, and then push it down
    Entry pop()
    {
        assert( not heap.empty() );

        if( heap.empty() ) {
            cerr << "Priority queue is empty!" << endl;
            exit( 1 );
        }

        Entry top = heap[0];
        heap[0]   = heap.back();
        heap.pop_back();

        if( not heap.empty() ) heapifyDown( 0 );

        return top;
    }

    // Remove an entry from the queue, identified by its value
    void remove( Valuetype value )
    {
        // Find the index of the entry with the given value
        auto it = find_if( heap.begin(), heap.end(), [value]( const Entry& entry ) { return entry.value == value; } );

        assert( it != heap.end() );

        if( it != heap.end() ) {
            int index = it - heap.begin();

            // Swap the entry with the last element and remove it
            *it = heap.back();
            heap.pop_back();

            // Re-heapify to maintain heap property
            // if( not heap.empty() ) heapifyDown( it - heap.begin() );
            if( not heap.empty() ) heapifyDown( index );
        }
    }

    // Set the priority of any given value
    void setPriority( Valuetype value, Prioritytype new_priority )
    {
        auto it = find_if( heap.begin(), heap.end(), [value]( const Entry& entry ) { return entry.value == value; } );

        assert( it != heap.end() );

        if( it != heap.end() ) {
            auto old_priority = it->priority;

            it->priority = new_priority;

            // Re-heapify to maintain heap property
            int index = it - heap.begin();

            // heapifyUp(index);
            // heapifyDown(index);

            if( old_priority > new_priority ) heapifyUp( index );
            if( old_priority < new_priority ) heapifyDown( index );

        } else {
            cerr << "Value not found in priority queue!" << endl;
            exit( 1 );
        }
    }
};

template<typename V, typename P>
void printEntryArray( const std::vector<typename PriorityQueue<V, P>::Entry>& entries )
{
    for( const auto& entry : entries ) {
        clog << entry.value << ':' << entry.priority << "\t";
    }
    clog << endl;
}

#endif