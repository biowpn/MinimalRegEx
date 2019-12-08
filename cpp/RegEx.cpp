
#include "RegEx.h"

#include <algorithm>
#include <iostream>
#include <unordered_map>

namespace RegEx
{

bool match(const char *pattern, const char *str)
{
    auto dfa = compile(pattern);
    return dfa(str);
}

DFA compile(const char *pattern)
{
    return DFA::from_regex(pattern);
}

template <class T>
bool in(const T &s, const std::vector<T> &vec)
{
    return std::find(vec.begin(), vec.end(), s) != vec.end();
}

DFA::transition_rule::transition_rule(unsigned _p, char _a, unsigned _q)
    : p(_p), a(_a), q(_q)
{
}

DFA::transition_rule_set::transition_rule_set(const uint_set &_p, char _a, const uint_set &_q)
    : p(_p), a(_a), q(_q)
{
}

class ParsingException : public std::exception
{
public:
    ParsingException(const char *msg) : m_msg(msg)
    {
    }

    const char *what() const throw() override
    {
        return m_msg;
    }

private:
    const char *m_msg;
};

DFA::DFA(unsigned s, const uint_set &F, const Rules_t &R) : m_s(s), m_F(F), m_R(R)
{
    m_num_states = 0;
    uint_set states;
    for (const auto &rule : m_R)
    {
        if (!states.has(rule.p))
        {
            states.add(rule.p);
            ++m_num_states;
        }
        if (!states.has(rule.q))
        {
            states.add(rule.q);
            ++m_num_states;
        }
    }

    reset();
}

DFA::DFA(uint_set s_set, const std::vector<uint_set> &F_set, const SRules_t &R_set)
{
    std::vector<uint_set> set_states;

    for (const auto &rule_set : R_set)
    {
        transition_rule rule;
        auto it_p = std::find(set_states.begin(), set_states.end(), rule_set.p);
        if (it_p == set_states.end())
        {
            rule.p = set_states.size();
            set_states.insert(it_p, rule_set.p);
        }
        else
        {
            rule.p = it_p - set_states.begin();
        }
        auto it_q = std::find(set_states.begin(), set_states.end(), rule_set.q);
        if (it_q == set_states.end())
        {
            rule.q = set_states.size();
            set_states.insert(it_q, rule_set.q);
        }
        else
        {
            rule.q = it_q - set_states.begin();
        }
        rule.a = rule_set.a;
        m_R.push_back(rule);
    }

    m_s = std::find(set_states.begin(), set_states.end(), s_set) - set_states.begin();

    for (const auto &f_set : F_set)
    {
        unsigned f = std::find(set_states.begin(), set_states.end(), f_set) - set_states.begin();
        m_F.add(f);
    }
}

void DFA::pclone(unsigned &s, uint_set &F, Rules_t &R, unsigned int offset) const
{
    s = m_s + offset * 8;
    F = m_F;
    F.rshift(offset);
    R.clear();
    for (const auto &rule : m_R)
    {
        R.push_back({rule.p + offset * 8, rule.a, rule.q + offset * 8});
    }
}

void DFA::E(const Rules_t &R, uint_set &states)
{
    bool done = false;
    while (!done)
    {
        done = true;
        for (const auto &rule : R)
        {
            if (rule.a == '\0' && states.has(rule.p) && !states.has(rule.q))
            {
                states.add(rule.q);
                done = false;
                break;
            }
        }
    }
}

DFA DFA::from_NFA(unsigned s, const uint_set &F, const Rules_t &R)
{
    uint_set s_set({s});
    std::vector<uint_set> F_set;
    SRules_t R_set;

    DFA::E(R, s_set);

    std::vector<uint_set> to_do({s_set});
    std::vector<uint_set> done;
    while (to_do.size() > 0)
    {
        uint_set p_set = to_do.back();
        to_do.pop_back();
        std::unordered_map<char, uint_set> cs;
        for (const auto &rule : R)
        {
            if (rule.a != '\0' && p_set.has(rule.p))
            {
                cs.emplace(rule.a, uint_set());
                cs[rule.a].add(rule.q);
            }
        }
        for (auto &kv : cs)
        {
            auto &q_set = kv.second;
            DFA::E(R, q_set);
            R_set.emplace_back(p_set, kv.first, q_set);
            if (q_set != p_set && !in(q_set, to_do) && !in(q_set, done))
            {
                to_do.push_back(q_set);
            }
        }
        done.push_back(p_set);
    }

    for (const auto &st_set : done)
    {
        if (st_set.intersect(F))
        {
            F_set.push_back(st_set);
        }
    }

    return DFA(s_set, F_set, R_set);
}

DFA DFA::from_regex(const char *regex)
{
    DFA e(0, {0}, {{0, '\0', 0}});
    if (*regex == '\0')
    {
        return e;
    }
    std::vector<Operator> op_stack;
    std::vector<DFA> v_stack;
    bool is_last_dfa = false;
    for (; *regex != '\0'; ++regex)
    {
        switch (*regex)
        {
        case static_cast<char>(Operator::left_parenthesis):
        {
            if (is_last_dfa)
            {
                op_stack.push_back(Operator::concatenation);
            }
            op_stack.push_back(Operator::left_parenthesis);
            is_last_dfa = false;
        }
        break;
        case static_cast<char>(Operator::right_parenthesis):
        {
            if (op_stack.size() > 0 && op_stack.back() == Operator::left_parenthesis)
            {
                v_stack.push_back(e);
            }
            while (op_stack.size() > 0 && op_stack.back() != Operator::left_parenthesis)
            {
                DFA::_eval(op_stack.back(), v_stack);
                op_stack.pop_back();
            }
            if (op_stack.size() > 0 && op_stack.back() == Operator::left_parenthesis)
            {
                op_stack.pop_back();
            }
            else
            {
                throw ParsingException("missing left parenthesis '('");
            }
        }
        break;
        case static_cast<char>(Operator::alternation):
        {
            while (op_stack.size() > 0 && (op_stack.back() == Operator::kleene_star || op_stack.back() == Operator::concatenation))
            {
                DFA::_eval(op_stack.back(), v_stack);
                op_stack.pop_back();
            }
            op_stack.push_back(Operator::alternation);
            is_last_dfa = false;
        }
        break;
        case static_cast<char>(Operator::kleene_star):
        {
            op_stack.push_back(Operator::kleene_star);
        }
        break;
        default:
        {
            auto dfa = DFA(0, {1}, {{0, *regex, 1}});
            if (is_last_dfa)
            {
                while (op_stack.size() > 0 && op_stack.back() == Operator::kleene_star)
                {
                    DFA::_eval(op_stack.back(), v_stack);
                    op_stack.pop_back();
                }
                op_stack.push_back(Operator::concatenation);
            }
            v_stack.push_back(dfa);
            is_last_dfa = true;
        }
        break;
        }
    }

    while (op_stack.size() > 0)
    {
        if (op_stack.back() == Operator::left_parenthesis)
        {
            throw ParsingException("missing right parenthesis ')'");
        }
        DFA::_eval(op_stack.back(), v_stack);
        op_stack.pop_back();
    }

    return v_stack.back();
}

void DFA::reset()
{
    m_state = m_s;
    m_trapped = false;
}

void DFA::advance(char a)
{
    for (const auto &rule : m_R)
    {
        if (rule.p == m_state && rule.a == a)
        {
            m_state = rule.q;
            return;
        }
    }
    m_trapped = true;
}

bool DFA::operator()(const char *tape)
{
    reset();
    while (*tape != '\0')
    {
        advance(*tape);
        if (m_trapped)
        {
            return false;
        }
        ++tape;
    }
    return m_F.has(m_state);
}

bool DFA::match(const char *string)
{
    return this->operator()(string);
}

DFA DFA::kleene_star() const
{
    unsigned s = m_R.size() * 2 + 1;
    auto F = m_F | uint_set({s});
    auto R = m_R;
    for (const auto &f : F)
    {
        R.push_back({f, '\0', m_s});
    }

    return DFA::from_NFA(s, F, R);
}

DFA DFA::concatenation(const DFA &other) const
{
    unsigned s_other;
    uint_set F_other;
    Rules_t R_other;
    other.pclone(s_other, F_other, R_other, m_num_states / 8 + 1);

    auto s = m_s;
    auto F = F_other;
    auto R = m_R;
    R.insert(R.end(), R_other.begin(), R_other.end());
    for (const auto &f : m_F)
    {
        R.push_back({f, '\0', s_other});
    }

    return DFA::from_NFA(s, F, R);
}

DFA DFA::alternation(const DFA &other) const
{
    unsigned s_other;
    uint_set F_other;
    Rules_t R_other;
    other.pclone(s_other, F_other, R_other, m_num_states / 8 + 1);

    unsigned s = other.m_num_states + (m_num_states / 8 + 1) * 8 + 1;
    auto F = m_F | F_other;
    auto R = m_R;
    R.insert(R.end(), R_other.begin(), R_other.end());
    R.emplace_back(s, '\0', m_s);
    R.emplace_back(s, '\0', s_other);

    return DFA::from_NFA(s, F, R);
}

void DFA::_eval(Operator op, std::vector<DFA> &v_stack) throw()
{
    switch (op)
    {
    case Operator::kleene_star:
    {
        if (v_stack.size() < 1)
        {
            throw ParsingException("missing operand for operator '*'");
        }
        v_stack.back() = v_stack.back().kleene_star();
        break;
    }
    case Operator::concatenation:
    {
        if (v_stack.size() < 2)
        {
            throw ParsingException("missing operand for operator '|'");
        }
        auto rhs = v_stack.back();
        v_stack.pop_back();
        auto lhs = v_stack.back();
        v_stack.pop_back();
        v_stack.push_back(lhs.concatenation(rhs));
        break;
    }
    case Operator::alternation:
    {
        if (v_stack.size() < 2)
        {
            throw ParsingException("missing operand for operator '|'");
        }
        auto rhs = v_stack.back();
        v_stack.pop_back();
        auto lhs = v_stack.back();
        v_stack.pop_back();
        v_stack.push_back(lhs.alternation(rhs));
        break;
    }
    default:
        throw ParsingException("unknown operator");
        break;
    }
}

} // namespace RegEx
