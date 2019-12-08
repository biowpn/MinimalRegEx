
#pragma once

#include "uint_set.hpp"

#include <vector>

namespace RegEx
{

class DFA
{
public:
    struct transition_rule
    {
        transition_rule() = default;
        transition_rule(unsigned, char, unsigned);
        unsigned p;
        char a;
        unsigned q;
    };

    struct transition_rule_set
    {
        transition_rule_set() = default;
        transition_rule_set(const uint_set &, char, const uint_set &);
        uint_set p;
        char a;
        uint_set q;
    };

    enum class Operator : char
    {
        kleene_star = '*',
        concatenation = '+',
        alternation = '|',
        left_parenthesis = '(',
        right_parenthesis = ')',
    };

    typedef std::vector<DFA::transition_rule> Rules_t;

    typedef std::vector<DFA::transition_rule_set> SRules_t;

    DFA(unsigned s, const uint_set &F, const Rules_t &R);

    DFA(uint_set s_set, const std::vector<uint_set> &F_set, const SRules_t &R_set);

    void pclone(unsigned &s, uint_set &F, Rules_t &R, unsigned int offset) const;

    static void E(const Rules_t &R, uint_set &states);

    static DFA from_NFA(unsigned s, const uint_set &F, const Rules_t &R);

    static DFA from_regex(const char *regex);

    void reset();

    void advance(char a);

    bool operator()(const char *tape);

    bool match(const char *string);

    DFA kleene_star() const;

    DFA concatenation(const DFA &other) const;

    DFA alternation(const DFA &other) const;

private:
    static void _eval(Operator op, std::vector<DFA> &v_stack) throw();

    unsigned m_s;
    uint_set m_F;
    Rules_t m_R;

    unsigned m_num_states;
    unsigned m_state;
    bool m_trapped;
};

bool match(const char *pattern, const char *str);

DFA compile(const char *pattern);

} // namespace RegEx
