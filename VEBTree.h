#pragma once

#include <memory>

struct VebViewConst {
    const unsigned char *D;
    unsigned int k;
    unsigned int M;

    bool empty() const;
    unsigned int succ(unsigned int) const;
    unsigned int pred(unsigned int) const;
};

struct VebView {
    unsigned char *D;
    unsigned int k;
    unsigned int M;

    operator VebViewConst() const { return VebViewConst{D, k, M}; }

    bool empty() const { return VebViewConst(*this).empty(); }
    void mkfull() const;
    void mkempty() const;
    void del(unsigned int) const;
    void put(unsigned int) const;
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

public:
    explicit VebTree(unsigned M, bool full);

    unsigned capacity() const { return M; }

    void del(unsigned x) { view().del(x); }
    void put(unsigned x) { view().put(x); }
    unsigned succ(unsigned x) const { return view().succ(x); }
    unsigned pred(unsigned x) const { return view().pred(x); }
};
