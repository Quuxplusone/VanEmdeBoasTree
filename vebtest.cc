#include "VEBTree.h"
#include <set>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

static std::vector<int> dump_forward(const VebTree& t)
{
    std::vector<int> result;
    for (int x = -1; true; ) {
        x = t.succ(x + 1);
        if (x == int(t.capacity())) break;
        result.push_back(x);
    }
    return result;
}

static std::vector<int> dump_backward(const VebTree& t)
{
    std::vector<int> result;
    for (int x = t.capacity(); true; ) {
        x = t.pred(x - 1);
        if (x == int(t.capacity())) break;
        result.push_back(x);
    }
    return result;
}

int main()
{
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
