#pragma once

#include <memory>

struct VebViewConst {
    const unsigned char *D;
    unsigned int k;
    unsigned int M;
};

struct VebView {
    unsigned char *D;
    unsigned int k;
    unsigned int M;

    operator VebViewConst() const { return VebViewConst{D, k, M}; }
};

void vebdel(VebView, unsigned int);
void vebput(VebView, unsigned int);
unsigned int vebsucc(VebViewConst, unsigned int);
unsigned int vebpred(VebViewConst, unsigned int);

struct VebTree {
    std::unique_ptr<unsigned char[]> D;
    unsigned int k;
    unsigned int M;

    VebView view() {
        return VebView{D.get(), k, M};
    }

    VebViewConst view() const {
        return VebViewConst{D.get(), k, M};
    }

    explicit VebTree(unsigned M, bool full);

    void del(unsigned x) { vebdel(view(), x); }
    void put(unsigned x) { vebput(view(), x); }
    void succ(unsigned x) const { vebsucc(view(), x); }
    void pred(unsigned x) const { vebpred(view(), x); }
};
