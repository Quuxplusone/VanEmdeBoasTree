#include "VEBTree.h"
#include <cassert>
#include <set>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

static std::vector<int> dump_forward(const VebTree& t)
{
    std::vector<int> result;
    for (int x : t) {
        result.push_back(x);
    }
    return result;
}

static std::vector<int> dump_backward(const VebTree& t)
{
    std::vector<int> result;
    for (auto it = t.rbegin(); it != t.rend(); ++it) {
        result.push_back(*it);
    }
    return result;
}

static void TestIssue1()
{
    VebTree t(513, false);
    std::set<int> expected;
    for (int i=0; i < 33; ++i) {
        t.put(i);
        expected.insert(i);
    }
    std::vector<int> actual = dump_backward(t);
    assert(std::equal(actual.begin(), actual.end(), expected.rbegin(), expected.rend()));
}

static void TestIssue2()
{
    VebTree t(32, false);
    std::set<int> expected;
    std::vector<int> actual;
    t.put(0);
    assert(t.pred(31) == 0);
    assert(t.pred(0) == 0);
    assert(t.pred(-1) == 32);
}

static void TestClear()
{
    VebTree t(100, false);
    assert(t.empty());
    for (int i=0; i < 10; ++i) {
        t.put(rand() % 100);
        assert(!t.empty());
    }
    t.clear();
    assert(t.empty());
}

int main()
{
    TestIssue1();
    TestIssue2();
    TestClear();

    int N = 500;
    int M = 100;

    VebTree t(N, false);

    std::set<int> values;
    for (int i=0; i < M; ++i) {
        int value = rand() % N;
        values.insert(value);
        t.put(value);
    }

    if (true) {
        auto v = dump_forward(t);
        if (!std::equal(v.begin(), v.end(), values.begin(), values.end())) {
            printf("ERROR! M=%d, N=%d\n", M, N);
            printf("expected={ ");
            for (int x : values) printf("%d ", x);
            printf("}\n");
            printf("actual={ ");
            for (int x : v) printf("%d ", x);
            printf("}\n");
        }
    }
    if (true) {
        auto v = dump_backward(t);
        if (!std::equal(v.begin(), v.end(), values.rbegin(), values.rend())) {
            printf("ERROR! M=%d, N=%d\n", M, N);
            printf("expected={ ");
            for (int x : values) printf("%d ", x);
            printf("}\n");
            printf("actual={ ");
            for (int x : v) printf("%d ", x);
            printf("}\n");
        }
    }
}
