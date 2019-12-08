

def match(pattern, string):
    dfa = compile(pattern)
    return dfa(string)


def compile(pattern):
    return DFA.from_regex(pattern)


class DFA:
    '''
    Deterministic Finite Automaton
    '''

    def __init__(self, S, Fs, Rs):
        '''
        @S initial state
        @Fs accepting states
        @Rs list of transition rules as (p, a, q), where
            p is the current state, (of type int)
            a is the symbol read,
            q is the next state (of type int)
        '''
        # enumerate states (convert them to int)
        # cannot use dict because states might not be hashable (such as set)
        states = []
        self.Rs = []
        for p, a, q in Rs:
            if p in states:
                p_int = states.index(p)
            else:
                p_int = len(states)
                states.append(p)
            if q in states:
                q_int = states.index(q)
            else:
                q_int = len(states)
                states.append(q)
            self.Rs.append((p_int, a, q_int))
        self.S = states.index(S)
        self.Fs = set([states.index(s) for s in Fs])

        self.num_states = len(states)

    def pclone(self, offset=0):
        S_ = self.S + offset
        Fs_ = {f + offset for f in self.Fs}
        Rs_ = []
        for p, a, q in self.Rs:
            Rs_.append([p + offset, a, q + offset])
        return S_, Fs_, Rs_

    @staticmethod
    def E(Rs, states):
        '''
        return the set of states reachable from any state in @states by reading ''
        '''
        reachables = set(states)
        done = False
        while not done:
            done = True
            for (p, a, q) in Rs:
                if p in reachables and a == '' and q not in reachables:
                    reachables.add(q)
                    done = False
                    break
        return reachables

    @staticmethod
    def from_NFA(S, Fs, Rs):
        '''
        construct a DFA from a (possibly) NFA
        '''
        S_ = DFA.E(Rs, {S})
        Rs_ = []
        Fs_ = []

        states_done = []
        states_to_do = [S_]
        while states_to_do:
            p_set = states_to_do.pop()
            cs = {}
            for p, a, q in Rs:
                if p in p_set and a != '':
                    if a not in cs:
                        cs[a] = set()
                    cs[a].add(q)
            for a, q_set in cs.items():
                q_set = DFA.E(Rs, q_set)
                Rs_.append((p_set, a, q_set))
                if q_set != p_set and q_set not in states_done and q_set not in states_to_do:
                    states_to_do.append(q_set)
            states_done.append(p_set)

        for f_set in states_done:
            if f_set.intersection(Fs):
                Fs_.append(f_set)

        return DFA(S_, Fs_, Rs_)

    def reset(self):
        self.state = self.S
        self.trapped = False

    def advance(self, symbol):
        for (p, a, q) in self.Rs:
            if self.state == p and a == symbol:
                self.state = q
                return
        self.trapped = True

    def __call__(self, tape):
        self.reset()
        for a in tape:
            self.advance(a)
            if self.trapped:
                return False
        return self.state in self.Fs

    def match(self, string):
        return self.__call__(string)

    def kleene_star(self):
        S_ = -1
        Fs_ = self.Fs.union({S_})
        Rs_ = [r for r in self.Rs]
        for f in Fs_:
            Rs_.append([f, '', self.S])

        return DFA.from_NFA(S_, Fs_, Rs_)

    def concatenation(self, other):
        # make sure self's states and other's states are disjoint
        S_other, Fs_other, Rs_other = other.pclone(self.num_states)

        S_ = self.S
        Fs_ = Fs_other
        Rs_ = self.Rs + Rs_other
        for f in self.Fs:
            Rs_.append((f, '', S_other))

        return DFA.from_NFA(S_, Fs_, Rs_)

    def alternation(self, other):
        # make sure self's states and other's states are disjoint
        S_other, Fs_other, Rs_other = other.pclone(self.num_states)

        S_ = -1
        Fs_ = set.union(self.Fs, Fs_other)
        Rs_ = self.Rs + Rs_other
        Rs_.append((S_, '', self.S))
        Rs_.append((S_, '', S_other))

        return DFA.from_NFA(S_, Fs_, Rs_)

    @staticmethod
    def _eval(op, v_stack):
        if op == '*':
            if len(v_stack) < 1:
                raise Exception("missing operand for operator '*'")
            lhs = v_stack.pop()
            v_stack.append(lhs.kleene_star())
        elif op == '+':
            if len(v_stack) < 2:
                raise Exception("missing operand for operator '+'")
            rhs = v_stack.pop()
            lhs = v_stack.pop()
            v_stack.append(lhs.concatenation(rhs))
        elif op == '|':
            if len(v_stack) < 2:
                raise Exception("missing operand for operator '|'")
            rhs = v_stack.pop()
            lhs = v_stack.pop()
            v_stack.append(lhs.alternation(rhs))
        else:
            raise Exception(f"undefined operator '{op}''")

    @staticmethod
    def from_regex(regex):
        '''
        construct DFA from regular expression
        '''
        e = DFA(0, {0}, [(0, '', 0)])
        if len(regex) == 0:
            return e
        # assume operator precedence: kleene_star > concatenation > alternation
        op_s = []
        dfa_s = []
        is_last_tk_dfa = False
        for a in regex:
            if a == '(':
                if is_last_tk_dfa:
                    op_s.append('+')
                op_s.append(a)
                is_last_tk_dfa = False
            elif a == ')':
                if op_s and op_s[-1] == '(':
                    dfa_s.append(e)
                while op_s and op_s[-1] != '(':
                    DFA._eval(op_s.pop(), dfa_s)
                if op_s and op_s[-1] == '(':
                    op_s.pop()
                else:
                    raise Exception("missing left parenthesis '('")
            elif a == '|':
                while op_s and op_s[-1] in ('*', '+', ):
                    DFA._eval(op_s.pop(), dfa_s)
                op_s.append(a)
                is_last_tk_dfa = False
            elif a == '*':
                op_s.append(a)
            else:
                dfa = DFA(0, {1}, [(0, a, 1)])
                if is_last_tk_dfa:
                    while op_s and op_s[-1] in ('*', ):
                        DFA._eval(op_s.pop(), dfa_s)
                    op_s.append('+')
                dfa_s.append(dfa)
                is_last_tk_dfa = True
        while op_s:
            op = op_s.pop()
            if op == '(':
                raise Exception("missing right parenthesis ')'")
            DFA._eval(op, dfa_s)

        return dfa_s[0]
