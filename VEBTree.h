#pragma once

#include <iterator>
#include <memory>

struct VebViewConst {
    const unsigned char *D;
    unsigned int k;
    unsigned int M;

    bool empty() const;
    unsigned int low() const;
    unsigned int high() const;
    unsigned int succ(unsigned int) const;
    unsigned int pred(unsigned int) const;
};

struct VebView {
    unsigned char *D;
    unsigned int k;
    unsigned int M;

    operator VebViewConst() const { return VebViewConst{D, k, M}; }

    bool empty() const { return VebViewConst(*this).empty(); }
    unsigned int low() const { return VebViewConst(*this).low(); }
    unsigned int high() const { return VebViewConst(*this).high(); }
    void mkfull() const;
    void mkempty() const;
    void setlow(unsigned int) const;
    void sethigh(unsigned int) const;
    void del(unsigned int) const;
    void put(unsigned int) const;
};

class VebTree;

class VebIterator {
    const VebTree *T;
    unsigned x;
public:
    using value_type = unsigned;
    using reference = unsigned;
    using pointer = unsigned*;
    using difference_type = decltype((int*)0 - (int*)0);
    using iterator_category = std::bidirectional_iterator_tag;

    VebIterator() noexcept = default;
    explicit VebIterator(const VebTree *T, unsigned x) noexcept : T(T), x(x) {}
    VebIterator& operator++();
    VebIterator operator++(int) { auto result(*this); ++*this; return result; }
    VebIterator& operator--();
    VebIterator operator--(int) { auto result(*this); --*this; return result; }
    unsigned operator*() const noexcept { return x; }

    friend bool operator==(const VebIterator& a, const VebIterator& b) noexcept {
        return a.T == b.T && a.x == b.x;
    }
    friend bool operator!=(const VebIterator& a, const VebIterator& b) noexcept {
        return !(a == b);
    }
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
    using const_iterator = VebIterator;
    using iterator = const_iterator;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    explicit VebTree(unsigned M, bool full);

    unsigned capacity() const { return M; }
    bool empty() const { return view().empty(); }
    void clear() { return view().mkempty(); }

    void del(unsigned x) { view().del(x); }
    void put(unsigned x) { view().put(x); }
    unsigned succ(unsigned x) const { return view().succ(x); }
    unsigned pred(unsigned x) const { return view().pred(x); }

    const_iterator begin() const { return const_iterator(this, view().low()); }
    const_iterator end() const { return const_iterator(this, capacity()); }
    const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }
    const_reverse_iterator rend() const { return const_reverse_iterator(begin()); }
};

inline VebIterator& VebIterator::operator++() { x = T->succ(x + 1); return *this; }
inline VebIterator& VebIterator::operator--() { x = T->pred(x - 1); return *this; }
