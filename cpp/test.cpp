
#include "RegEx.h"

#include <assert.h>
#include <bitset>
#include <iostream>
#include <string>
#include <sstream>

void test_basic()
{
    auto dfa = RegEx::DFA(0, {2}, {
                                      {0, 'b', 1},
                                      {1, 'b', 2},
                                      {1, 'a', 3},
                                      {2, 'b', 2},
                                      {2, 'a', 3},
                                      {3, 'a', 3},
                                      {3, 'b', 2},
                                  });
    // b(a|b)*b

    assert(dfa("") == false);
    assert(dfa("b") == false);
    assert(dfa("bb") == true);
    assert(dfa("bbb") == true);
    assert(dfa("ba") == false);
    assert(dfa("ab") == false);
    assert(dfa("bab") == true);
    assert(dfa("baabbaabab") == true);
}

void test_nfa_to_dfa()
{
    auto dfa = RegEx::DFA::from_NFA(0, {2}, {
                                                {0, 'a', 0},
                                                {0, 'b', 0},
                                                {0, '\0', 1},
                                                {1, 'b', 2},
                                            });
    // (a|b)*b

    assert(dfa("a") == false);
    assert(dfa("b") == true);
    assert(dfa("ab") == true);
    assert(dfa("bb") == true);
    assert(dfa("aba") == false);
    assert(dfa("abbbaabaab") == true);
}

void test_kleene_star()
{
    auto dfa1 = RegEx::DFA(0, {2}, {
                                       {0, 'a', 1},
                                       {1, 'b', 2},
                                   });
    // ab

    auto dfa2 = dfa1.kleene_star();
    // (ab)*

    assert(dfa1("a") == false);
    assert(dfa1("b") == false);
    assert(dfa1("ab") == true);
    assert(dfa1("ba") == false);
    assert(dfa1("ababab") == false);
    assert(dfa2("ababab") == true);
    assert(dfa2("abaaab") == false);
    assert(dfa2("") == true);
}

void test_concat()
{
    auto dfa3 = RegEx::DFA(0, {2}, {
                                       {0, 'a', 1},
                                       {1, 'b', 2},
                                   });
    // ab

    auto dfa4 = RegEx::DFA(0, {2}, {
                                       {0, 'b', 1},
                                       {1, 'a', 2},
                                   });
    // ba

    auto dfa5 = dfa3.concatenation(dfa4);
    // abba

    assert(dfa5("ab") == false);
    assert(dfa5("ba") == false);
    assert(dfa5("abba") == true);
    assert(dfa5("baab") == false);
    assert(dfa5("abbba") == false);
}

void test_union()
{
    auto dfa6 = RegEx::DFA(0, {3}, {
                                       {0, 'b', 1},
                                       {1, 'a', 2},
                                       {2, 'b', 3},
                                   });
    // bab

    auto dfa7 = RegEx::DFA(0, {3}, {
                                       {0, 'b', 1},
                                       {1, 'b', 2},
                                       {2, 'a', 3},
                                   });
    // bba

    auto dfa8 = dfa6.alternation(dfa7);
    // bab|bba

    assert(dfa8("ab") == false);
    assert(dfa8("ba") == false);
    assert(dfa8("bab") == true);
    assert(dfa8("bba") == true);
    assert(dfa8("baa") == false);
    assert(dfa8("bbb") == false);
}

void test_regex()
{
    auto dfa = RegEx::DFA::from_regex("b(a|b)*b");
    assert(dfa("") == false);
    assert(dfa("b") == false);
    assert(dfa("bb") == true);
    assert(dfa("bab") == true);
    assert(dfa("ba") == false);
    assert(dfa("bab") == true);
    assert(dfa("bababbaab") == true);

    // empty string
    auto e = RegEx::DFA::from_regex("");
    assert(e("") == true);
    assert(e("a") == false);
    // "()" can specify empty string too
    auto e2 = RegEx::DFA::from_regex("()");
    assert(e2("") == true);
    assert(e2("a") == false);

    // emulate '?' operator
    // though I could, I choose not to make "(|abc)" legal
    auto zero_or_one = RegEx::DFA::from_regex("(()|abc)");
    assert(zero_or_one("") == true);
    assert(zero_or_one("abc") == true);
    assert(zero_or_one("abcabc") == false);

    // binary divisible by 3
    // https://stackoverflow.com/a/19608040/10899376
    auto div3 = RegEx::DFA::from_regex("(1(01*0)*1|0)*");
    for (unsigned i = 0; i < 100; ++i)
    {
        std::stringstream ss;
        ss << std::bitset<32>(i);
        assert(div3(ss.str().c_str()) == (i % 3 == 0));
    }
}

int main()
{
    std::cout << "testing basic" << std::endl;
    test_basic();

    std::cout << "testing NFA to DFA conversion" << std::endl;
    test_nfa_to_dfa();

    std::cout << "testing operation kleene star" << std::endl;
    test_kleene_star();

    std::cout << "testing operation concatenation" << std::endl;
    test_concat();

    std::cout << "testing operation union" << std::endl;
    test_union();

    std::cout << "testing regular expression to DFA conversion" << std::endl;
    test_regex();

    std::cout << "all passed" << std::endl;

    return 0;
}