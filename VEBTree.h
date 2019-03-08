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

class VebTree {
    std::unique_ptr<unsigned char[]> D;
    unsigned int k;
    unsigned int M;

    VebView view() {
        return VebView{D.get(), k, M};
    }

    VebViewConst view() const {
        return VebViewConst{D.get(), k, M};
    }

    static void vebmkfull(VebView T);
    static void vebdel(VebView, unsigned int);
    static void vebput(VebView, unsigned int);
    static unsigned int vebsucc(VebViewConst, unsigned int);
    static unsigned int vebpred(VebViewConst, unsigned int);

public:
    explicit VebTree(unsigned M, bool full);

    unsigned capacity() const { return M; }

    void del(unsigned x) { vebdel(view(), x); }
    void put(unsigned x) { vebput(view(), x); }
    unsigned succ(unsigned x) const { return vebsucc(view(), x); }
    unsigned pred(unsigned x) const { return vebpred(view(), x); }
};
