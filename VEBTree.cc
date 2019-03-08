#include <stdlib.h>
#include <memory>
#include "VEBTree.h"

typedef unsigned int uint;

#define BITS_PER_WORD (8 * sizeof(unsigned int))

static uint count_leading_zeros(uint x)
{
    return __builtin_clz(x);
}

static uint count_trailing_zeros(uint x)
{
    return __builtin_ctz(x);
}

static uint bytes(uint x)
{
    return x/8 + (x%8>0);
}

static uint zeros(uint k)
{
    return ~0<<k;
}

static uint ones(uint k)
{
    if (k == BITS_PER_WORD) {
        return -1u;
    }
    return ~zeros(k);
}

static uint lowbits(uint x, uint k)
{
    return x&ones(k);
}

static uint highbits(uint x, uint k)
{
    return x>>k;
}

static uint
decode(const unsigned char *D, uint b)
{
    uint x = 0;
    for (uint i = 0; i < b; ++i) {
        x |= D[i] << (8*i);
    }
    return x;
}

static void
encode(unsigned char *D, uint b, uint x)
{
    for (uint i = 0; i < b; ++i) {
        D[i] = (x >> (8*i));
    }
}

static void
set(unsigned char *D, uint x)
{
    D[x/8] |= 1u << (x % 8);
}

static void
unset(unsigned char *D, uint x)
{
    D[x/8] &= ~(1u << (x % 8));
}

uint VebViewConst::low() const
{
    VebViewConst T = *this;
    if (T.M <= BITS_PER_WORD) {
        uint x = decode(T.D, bytes(T.M));
        if (x == 0) {
            return T.M;
        }
        return count_trailing_zeros(x);
    }
    return decode(T.D, bytes(T.k));
}

void VebView::setlow(uint x) const
{
    VebView T = *this;
    if (T.M <= BITS_PER_WORD) {
        set(T.D, x);
    } else {
        encode(T.D, bytes(T.k), x);
    }
}

uint VebViewConst::high() const
{
    VebViewConst T = *this;
    if (T.M <= BITS_PER_WORD) {
        uint x = decode(T.D, bytes(T.M));
        if (x == 0) {
            return T.M;
        }
        return BITS_PER_WORD - count_leading_zeros(x) - 1;
    }
    return decode(T.D + bytes(T.k), bytes(T.k));
}

void VebView::sethigh(uint x) const
{
    VebView T = *this;
    if (T.M <= BITS_PER_WORD) {
        set(T.D, x);
    } else {
        encode(T.D + bytes(T.k), bytes(T.k), x);
    }
}

unsigned int vebsize(unsigned int M)
{
    if (M <= BITS_PER_WORD) {
        return bytes(M);
    }
    uint k = BITS_PER_WORD - count_leading_zeros(M - 1);
    uint m = highbits(M-1, k/2) + 1;
    uint n = 1u << (k/2);
    return 2*bytes(k) + vebsize(m) + (m-1)*vebsize(n) + vebsize(M-(m-1)*n);
}

template<class VV>
static VV aux(VV S)
{
    VV T;
    T.k = S.k - S.k/2;
    T.D = S.D + 2*bytes(S.k);
    T.M = highbits(S.M-1, S.k/2) + 1;
    return T;
}

template<class VV>
static VV branch(VV S, uint i)
{
    VV T;
    uint k = S.k/2;
    uint m = highbits(S.M-1, k) + 1;
    uint n = 1u << k;
    if (i < m-1) {
        T.M = n;
        T.k = k;
    } else {
        T.M = S.M - (m-1)*n;
        T.k = BITS_PER_WORD - count_leading_zeros(T.M - 1);
    }
    T.D = S.D + 2*bytes(S.k) + vebsize(m) + i*vebsize(n);
    return T;
}

bool VebViewConst::empty() const
{
    VebViewConst T = *this;
    if (T.M <= BITS_PER_WORD) {
        return decode(T.D, bytes(T.M)) == 0;
    }
    return (T.low() > T.high());
}

void VebView::mkempty() const
{
    VebView T = *this;
    if (T.M <= BITS_PER_WORD) {
        encode(T.D, bytes(T.M), 0);
        return;
    }
    T.setlow(1);
    T.sethigh(0);
    aux(T).mkempty();
    uint m = highbits(T.M-1, T.k/2) + 1;
    for (uint i = 0; i < m; ++i) {
        branch(T, i).mkempty();
    }
}

void VebView::mkfull() const
{
    VebView T = *this;
    if (T.M <= BITS_PER_WORD) {
        encode(T.D, bytes(T.M), ones(T.M));
        return;
    }
    T.setlow(0);
    T.sethigh(T.M-1);
    aux(T).mkfull();
    uint m = highbits(T.M-1, T.k/2) + 1;
    for (uint i = 0; i < m; ++i) {
        VebView B = branch(T, i);
        B.mkfull();
        if (i == 0) {
            B.del(0);
        }
        if (i == m-1) {
            B.del(lowbits(T.M-1, T.k/2));
        }
    }
}

VebTree::VebTree(unsigned M, bool full)
{
    this->k = BITS_PER_WORD - count_leading_zeros(M - 1);
    this->D = std::make_unique<unsigned char[]>(vebsize(M));
    this->M = M;
    if (full) {
        this->view().mkfull();
    } else {
        this->view().mkempty();
    }
}

void VebView::put(unsigned int x) const
{
    VebView T = *this;
    if (x >= T.M)
        return;
    if (T.M <= BITS_PER_WORD) {
        set(T.D, x);
        return;
    }
    if (T.empty()) {
        T.setlow(x);
        T.sethigh(x);
        return;
    }
    uint lo = T.low();
    uint hi = T.high();
    if (x == lo || x == hi) {
        return;
    }
    if (x < lo) {
        T.setlow(x);
        if (lo == hi) {
            return;
        }
        x = lo;
    } else if (x > hi) {
        T.sethigh(x);
        if (lo == hi) {
            return;
        }
        x = hi;
    }
    uint i = highbits(x, T.k/2);
    uint j = lowbits(x, T.k/2);
    VebView B = branch(T, i);
    B.put(j);
    if (B.low() == B.high()) {
        aux(T).put(i);
    }
}

void VebView::del(unsigned int x) const
{
    VebView T = *this;
    if (T.empty() || x >= T.M) {
        return;
    }
    if (T.M <= BITS_PER_WORD) {
        unset(T.D, x);
        return;
    }
    uint lo = T.low();
    uint hi = T.high();
    if (x < lo || x > hi) {
        return;
    }
    if (lo == hi && x == lo) {
        T.sethigh(0);
        T.setlow(1);
        return;
    }
    uint i;
    uint j;
    VebView B;
    VebView A = aux(T);
    if (x == lo) {
        if (A.empty()) {
            T.setlow(hi);
            return;
        } else {
            i = A.low();
            B = branch(T, i);
            j = B.low();
            T.setlow((i << (T.k/2)) + j);
        }
    } else if (x == hi) {
        if (A.empty()) {
            T.sethigh(lo);
            return;
        } else {
            i = A.high();
            B = branch(T, i);
            j = B.high();
            T.sethigh((i << (T.k/2)) + j);
        }
    } else {
        i = highbits(x, T.k/2);
        j = lowbits(x, T.k/2);
        B = branch(T, i);
    }
    B.del(j);
    if (B.empty()) {
        A.del(i);
    }
}

unsigned int VebViewConst::succ(unsigned int x) const
{
    VebViewConst T = *this;
    uint hi = T.high();
    if (T.empty() || x > hi) {
        return T.M;
    }
    if (T.M <= BITS_PER_WORD) {
        uint y = decode(T.D, bytes(T.M));
        y &= zeros(x);
        if (y > 0) {
            return count_trailing_zeros(y);
        }
        return T.M;
    }
    uint lo = T.low();
    if (x <= lo) {
        return lo;
    }
    VebViewConst A = aux(T);
    if (A.empty() || x == hi) {
        return hi;
    }
    uint i = highbits(x, T.k/2);
    uint j = lowbits(x, T.k/2);
    VebViewConst B = branch(T, i);
    if (!B.empty() && j <= B.high()) {
        return (i << (T.k/2)) + B.succ(j);
    }
    i = A.succ(i + 1);
    if (i == A.M) {
        return hi;
    }
    B = branch(T, i);
    return (i << (T.k/2)) + B.low();
}

unsigned int VebViewConst::pred(unsigned int x) const
{
    VebViewConst T = *this;
    uint lo = T.low();
    if (T.empty() || x < lo || x >= T.M) {
        return T.M;
    }
    if (T.M <= BITS_PER_WORD) {
        uint y = decode(T.D, bytes(T.M));
        y &= ones(x + 1);
        if (y != 0) {
            return BITS_PER_WORD - count_leading_zeros(y) - 1;
        }
        return T.M;
    }
    uint hi = T.high();
    if (x >= hi) {
        return hi;
    }
    VebViewConst A = aux(T);
    if (A.empty() || x == lo) {
        return lo;
    }
    uint i = highbits(x, T.k/2);
    uint j = lowbits(x, T.k/2);
    VebViewConst B = branch(T, i);
    if (!B.empty() && j >= B.low()) {
        return (i << (T.k/2)) + B.pred(j);
    }
    i = A.pred(i-1);
    if (i == A.M) {
        return lo;
    }
    B = branch(T, i);
    return (i << (T.k/2)) + B.high();
}
