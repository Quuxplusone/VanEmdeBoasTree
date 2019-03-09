#include "VEBTree.h"
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

void test(int N, int M, bool full)
{
    printf("N=%d M=%d full=%d\n", N, M, int(full));
    VebTree t(N, full);

    std::set<int> values;
    if (full) {
        for (int i=0; i < N; ++i) {
            values.insert(i);
        }
    } else {
        for (int i=0; i < M; ++i) {
            int value = rand() % N;
            values.insert(value);
            t.put(value);
            printf("insert %d\n", value);
        }
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
            abort();
        }
    }
    if (true) {
        auto v = dump_backward(t);
        if (!std::equal(v.begin(), v.end(), values.rbegin(), values.rend())) {
            printf("ERROR! M=%d, N=%d, reverse iteration\n", M, N);
            printf("expected={ ");
            for (int x : values) printf("%d ", x);
            printf("}\n");
            printf("actual={ ");
            for (int x : v) printf("%d ", x);
            printf("}\n");
            abort();
        }
    }
}

int main()
{
    for (int M = 10; M < 100; ++M) {
        test(M, 0, true);
        for (int N = M; N < 100; ++N) {
            test(N, M, false);
        }
    }
    puts("Done.");
}
