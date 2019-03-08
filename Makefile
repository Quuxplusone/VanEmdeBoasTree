
vebtest: vebtest.cc VEBTree.cc VEBTree.h
	$(CXX) -W -Wall -Wextra -std=c++14 $(CXXFLAGS) vebtest.cc VEBTree.cc -o vebtest

vebtest-exhaustive: vebtest-exhaustive.cc VEBTree.cc VEBTree.h
	$(CXX) -W -Wall -Wextra -std=c++14 $(CXXFLAGS) vebtest-exhaustive.cc VEBTree.cc -o vebtest-exhaustive
