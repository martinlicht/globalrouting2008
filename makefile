
default: all 


CC := clang++ -O3 -std=c++20 -g -Wall -Wextra -pedantic -Wno-sign-compare -Wnarrowing 

# List all .hpp files in the directory
HEADERS := $(wildcard *.hpp)

# Rule to check each file
%.hpp:
	@$(CC) -fsyntax-only $@
	@echo "Checked $@"


.PHONY: headerchecks all format clean

headerchecks: $(HEADERS)
	@echo "Syntax check completed."

format:
	clang-format -i common.hpp
	clang-format -i grp.hpp
	# clang-format -i grp2graph.hpp
	# clang-format -i graph.hpp
	# clang-format -i connector.hpp
	clang-format -i priority_queue.hpp
	clang-format -i output_tree.hpp
	clang-format -i main.cpp

%.out: common.hpp
	@echo "Rebuilding $@ because of changes in $<"

test_priority_queue.out: priority_queue.hpp test_priority_queue.cpp common.hpp
	$(CC) test_priority_queue.cpp -o test_priority_queue.out 

test_grp.out: grp.hpp test_grp.cpp  common.hpp
	$(CC) test_grp.cpp -o test_grp.out 

test_graph.out: graph.hpp test_graph.cpp  common.hpp
	$(CC) test_graph.cpp -o test_graph.out 

test_grp2graph.out: grp2graph.hpp test_grp2graph.cpp  common.hpp
	$(CC) test_grp2graph.cpp -o test_grp2graph.out 

debug_main.out: main.cpp priority_queue.hpp grp.hpp graph.hpp grp2graph.hpp connector.hpp output_tree.hpp common.hpp
	$(CC) -D_GLIBCXX_DEBUG main.cpp -o debug_main.out 

main.out:       main.cpp priority_queue.hpp grp.hpp graph.hpp grp2graph.hpp connector.hpp output_tree.hpp common.hpp
	$(CC) -DNDEBUG main.cpp -o main.out 

all: test_priority_queue.out test_grp.out test_graph.out test_grp2graph.out main.out debug_main.out


.PHONY: data evaluationscript

evaluationscript:
	wget http://www.ispd.cc/contests/08/eval2008.zip
	unzip "eval2008.zip"
	chmod +x eval2008/eval2008.pl 

data:
	wget http://www.ispd.cc/contests/07/rcontest/benchmark/adaptec1.capo70.2d.35.50.90.gr.gz
	wget http://www.ispd.cc/contests/07/rcontest/benchmark/adaptec1.capo70.3d.35.50.90.gr.gz
	wget http://www.ispd.cc/contests/07/rcontest/benchmark/adaptec2.mpl60.3d.35.20.100.gr.gz
	wget http://www.ispd.cc/contests/07/rcontest/benchmark/adaptec3.dragon70.3d.30.50.90.gr.gz
	wget http://www.ispd.cc/contests/07/rcontest/benchmark/adaptec4.aplace60.3d.30.50.90.gr.gz
	wget http://www.ispd.cc/contests/07/rcontest/benchmark/adaptec1.capo70.3d.35.50.90.gr.gz
	wget http://www.ispd.cc/contests/07/rcontest/benchmark/adaptec5.mfar50.3d.50.20.100.gr.gz
	wget http://www.ispd.cc/contests/07/rcontest/benchmark/newblue1.ntup50.3d.30.50.90.gr.gz
	wget http://www.ispd.cc/contests/07/rcontest/benchmark/newblue2.fastplace90.3d.50.20.100.gr.gz
	wget http://www.ispd.cc/contests/07/rcontest/benchmark/newblue3.kraftwerk80.3d.40.50.90.gr.gz
	wget http://www.ispd.cc/contests/08/benchmark/bigblue1.capo60.3d.50.10.100.gr.gz
	wget http://www.ispd.cc/contests/08/benchmark/bigblue2.mpl60.3d.40.60.60.gr.gz
	wget http://www.ispd.cc/contests/08/benchmark/bigblue3.aplace70.3d.50.10.90.m8.gr.gz
	wget http://www.ispd.cc/contests/08/benchmark/bigblue4.fastplace70.3d.80.20.80.gr.gz
	wget http://www.ispd.cc/contests/08/benchmark/newblue4.mpl50.3d.40.10.95.gr.gz
	wget http://www.ispd.cc/contests/08/benchmark/newblue5.ntup50.3d.40.10.100.gr.gz
	wget http://www.ispd.cc/contests/08/benchmark/newblue6.mfar80.3d.60.10.100.gr.gz
	wget http://www.ispd.cc/contests/08/benchmark/newblue7.kraftwerk70.3d.80.20.82.m8.gr.gz
	for file in *.gz; do gunzip "$$file"; done


clean:
	rm -f *.gch *.o *.out



