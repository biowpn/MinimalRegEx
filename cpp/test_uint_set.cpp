
#include "uint_set.hpp"

#include "assert.h"
#include <iostream>

void test_ctor_dtor()
{
    // default ctor
    {
        uint_set A;
    }
    // initializer list + copy ctor
    {
        uint_set A{1, 2, 3, 30, 60, 300};
        {
            uint_set B(A);
        }
    }
    // assignment operator
    {
        uint_set A{1, 2, 3, 30, 60, 300};
        {
            uint_set B{7, 8, 9};
            B = A;
        }
    }
}

void test_equality()
{
    uint_set A{1, 10, 20, 50, 600};
    uint_set B{600, 50, 20, 10, 1};
    uint_set C{1, 10, 20, 50};
    uint_set D{1, 10, 20, 50, 600, 601};
    uint_set E{1, 10, 20, 50, 601};

    assert(uint_set() == uint_set({}));
    assert(A == A);
    assert(A == uint_set(A));
    assert(A == B);
    assert(B == A);
    assert(A != C);
    assert(C != A);
    assert(A != D);
    assert(D != A);
    assert(A != E);
    assert(E != A);
}

void test_has()
{
    uint_set A{103, 106, 109, 112, 115, 118};
    unsigned nums[] = {103, 106, 109, 112, 115, 118};
    for (unsigned n = 0; n < nums[0]; ++n)
    {
        assert(!A.has(n));
    }
    for (auto i = 0; i < sizeof(nums) / sizeof(nums[0]); ++i)
    {
        assert(A.has(nums[i]));
        assert(!A.has(nums[i] - 1));
        assert(!A.has(nums[i] + 1));
    }
}

void test_add()
{
    uint_set A{};
    for (unsigned n = 100; n < 1000; n += 3)
    {
        A.add(n);
    }
    for (unsigned n = 100; n < 1000; n += 3)
    {
        assert(A.has(n));
        assert(!A.has(n - 1));
        assert(!A.has(n + 1));
    }
}

void test_remove_empty()
{
    uint_set A;
    for (unsigned n = 100; n < 1000; n += 5)
    {
        assert(A.empty());
        A.add(n);
        assert(!A.empty());
        A.remove(n);
    }
    assert(A.empty());
}

void test_set_operation()
{
    uint_set A;
    uint_set B;
    uint_set AB_union;
    uint_set AB_intersection;

    for (unsigned n = 0; n < 1000; n += 2)
    {
        A.add(n);
        AB_union.add(n);
    }
    for (unsigned n = 0; n < 1000; n += 3)
    {
        B.add(n);
        AB_union.add(n);
    }
    for (unsigned n = 0; n < 1000; n += 6)
    {
        AB_intersection.add(n);
    }

    uint_set s1 = (A | B);
    uint_set s2 = (A & B);
    assert(s1 == AB_union);
    assert(s2 == AB_intersection);
}

void test_rshift()
{
    uint_set A;
    uint_set B;
    for (unsigned n = 100; n < 200; ++n)
    {
        A.add(n);
        B.add(n + 16);
    }
    A.rshift(2);
    assert(A == B);
}

void test_iteration()
{
    uint_set A;
    uint_set B;
    for (unsigned n = 100; n < 200; n += 3)
    {
        A.add(n);
    }
    for (const auto a : A)
    {
        B.add(a);
    }
    assert(A == B);
}

void test_extra()
{
    // rshift + union
    {
        uint_set A{3, 5};
        uint_set B(A);
        B.rshift(1);
        uint_set C({3, 5, 11, 13});
        assert((A | B) == C);
    }

    // A | B = (A - B) + (B - A) + A & B
    {
        uint_set A{100, 200, 300};
        uint_set B{200, 300, 400};
        uint_set A_minus_B(A);
        uint_set B_minus_A(B);
        for (const auto &b : B)
        {
            A_minus_B.remove(b);
        }
        for (const auto &a : A)
        {
            B_minus_A.remove(a);
        }
        uint_set s1 = A | B;
        uint_set s2 = A & B;
        assert(s1 == (s2 | A_minus_B | B_minus_A));
    }
}

int main()
{
    std::cout << "test starts" << std::endl;

    test_ctor_dtor();
    test_equality();
    test_has();
    test_add();
    test_remove_empty();
    test_set_operation();
    test_rshift();
    test_iteration();
    test_extra();

    std::cout << "test completed" << std::endl;

    return 0;
}
