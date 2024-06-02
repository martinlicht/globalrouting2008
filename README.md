# GlobalRouting2008

This project computes approximate solutions of global routing instances, as in the ISPD Global Routing Contest 2008.
The code is based on a C++ programming-lab at the University of Bonn, but the code has been completely modernized for the purpose of this repository. 

Global routing is a notoriously hard combinatorial problem. The routing algorithm is rather simple and demonstrates a rudimentary approach to global routing. A basic outline is this:
- Each net is routed by selecting one of its pins and searching the other pins using Dijkstra's algorithm
- The routing of each net has two possible phases: 
  1. Search a solution within a bounding box of the pin only and respect capacity bounds
  2. If that fails, then relax the handling of capacity bounds and search the entire graph
- The routing starts at the node closest to the weighted center of the pins 
- Experience suggests that routing nets with fewer pins first improves performance

Trying to route nets throughout the entire graph, one after another, while *always* respecting capacity bounds might generally fail:
as the graph becomes congested, it might be impossible to connect some nets without overflowing the capacity of some edges.
That is the reason for a second phase with relaxed capacity consideration.

## Installation and usage

The compilation requires `git`, a C++20 compiler (such as `gcc`), and `make`. 
To clone and compile the code (debug version and normal version):

```bash
git clone https://github.com/martinlicht/globalrouting2008.git
cd ./globalrouting2008
make all
```

To download the evaluation script of the 2008 global routing contest and the data:

```bash
make evaluationscript 
make data 
```

Running the main executbale `main.out` or its debug version `debug_main.out` for any given input file, say, `instance.gr`, will produce a solution file `instance.gr.solution`. 
Please see below for details on the file format. 

```
$main.out instance.gr
```

Next, you can evaluate the solution using the evaluation Perl script, as in:

```
$./eval2008/eval2008.pl instance.gr instance.gr.solution
```


# Input file format

The input files are composed of the following sections:

```
grid n_x n_y l
vertical capacity c_1 c_2 ... c_l
horizontal capacity d_1 d_2 ... d_l
minimum width m_1 m_2 ... m_l
minimum spacing s_1 s_2 ... s_l
via spacing v_1 v_2 ... v_l
b_x b_y t_x t_y
```

This part consists of keywords and the following variables:
 - n_x and n_y are the number of tiles in the horizontald and vertical direction, and l is the number of layers 
 - c_1 c_2 ... c_l and d_1 d_2 ... d_l are the capacities in the horizontal and vertical directions, respectively, one such number for each layer
 - m_1 m_2 ... m_l, s_1 s_2 ... s_l, v_1 v_2 ... v_l are minimum widths, minimum spacings, and via spacings for each layer
 - b_x and b_y are the lower left coordinate corner of the grid
 - t_x and t_y are the width and height of each tile 

This is followed by the description of the nets:

```
num net N
```

Here, N is the number of nets, and is followed by the corresponding number of nets, which have the following format:

```
netname k n_p M
p_x p_y p_z
p_x p_y p_z
...
```

The netname is an alphanumerical string (without spaces), k is the index of the net, and n_p is the number of pins in that net, and M is the minimum width of the net.
The pins are given by their horizontal and vertical position p_x, p_y and by their layer p_z. Note that p_z is a one-based index.

The last section is composed of capacity adjustments and has the following form:
```
N_a
c_1 r_1 l_1 c_2 r_2 l_2 f
....
```

Here, N_a is the number of capacity adjustments, and is followed by the corresponding number of adjustment specifications. 
Each of the following lines contains the column-row-layer positions of the two tiles and the adjusted capacity between the two tiles. 
Notice that adjustments only happen for tiles on the same layer, as there are no adjustments across layers.

# Output file format 

The output file contains of records that describe the spanning tree for each net 

```
netname k n_e
(p_x,p_y,p_z)-(q_x,q_y,q_z)
(p_x,p_y,p_z)-(q_x,q_y,q_z)
...
(p_x,p_y,p_z)-(q_x,q_y,q_z)
!
...
```

Here, *netname* and k are the name and index of the net, n_e denotes the number of edges, and each line contains the physical positions (p_x,p_y,p_z) and (q_x,q_y,q_z) of the start and end of a connection across tiles and layers. 
Once more, notice that the layer indices p_z and q_z are one-based. 


## License

This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details

## Status

This project is based on a programming lab from many years ago and I make contributions in my free time.

