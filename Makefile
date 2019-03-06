
test: test.cc VEBTree.cc VEBTree.h
	$(CXX) -W -Wall -Wextra -std=c++14 $(CXXFLAGS) test.cc VEBTree.cc
