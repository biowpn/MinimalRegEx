
import RegEx


def test_basic():
    dfa = RegEx.DFA(0, {2}, [
        [0, 'b', 1],
        [1, 'b', 2],
        [1, 'a', 3],
        [2, 'b', 2],
        [2, 'a', 3],
        [3, 'a', 3],
        [3, 'b', 2]
    ])
    # b(a|b)*b

    assert (dfa("") == False)
    assert (dfa("b") == False)
    assert (dfa("bb") == True)
    assert (dfa("bbb") == True)
    assert (dfa("ba") == False)
    assert (dfa("ab") == False)
    assert (dfa("bab") == True)
    assert (dfa("baabbaabab") == True)


def test_nfa_to_dfa():
    dfa = RegEx.DFA.from_NFA(0, {2}, [
        (0, 'a', 0),
        (0, 'b', 0),
        (0, '', 1),
        (1, 'b', 2),
    ])
    # (a|b)*b

    assert (dfa("a") == False)
    assert (dfa("b") == True)
    assert (dfa("ab") == True)
    assert (dfa("bb") == True)
    assert (dfa("aba") == False)
    assert (dfa("abbbaabaab") == True)


def test_kleene_star():
    dfa1 = RegEx.DFA(0, {2}, [
        [0, 'a', 1],
        [1, 'b', 2],
    ])
    # ab

    dfa2 = dfa1.kstar()
    # (ab)*

    assert (dfa1("a") == False)
    assert (dfa1("b") == False)
    assert (dfa1("ab") == True)
    assert (dfa1("ba") == False)
    assert (dfa1("ababab") == False)
    assert (dfa2("ababab") == True)
    assert (dfa2("abaaab") == False)
    assert (dfa2("") == True)


def test_concat():
    dfa3 = RegEx.DFA(0, {2}, [
        [0, 'a', 1],
        [1, 'b', 2],
    ])
    # ab

    dfa4 = RegEx.DFA(0, {2}, [
        [0, 'b', 1],
        [1, 'a', 2],
    ])
    # ba

    dfa5 = dfa3.concat(dfa4)
    # abba

    assert(dfa5("ab") == False)
    assert(dfa5("ba") == False)
    assert(dfa5("abba") == True)
    assert(dfa5("baab") == False)
    assert(dfa5("abbba") == False)


def test_union():
    dfa6 = RegEx.DFA(0, {3}, [
        [0, 'b', 1],
        [1, 'a', 2],
        [2, 'b', 3],
    ])
    # bab

    dfa7 = RegEx.DFA(0, {3}, [
        [0, 'b', 1],
        [1, 'b', 2],
        [2, 'a', 3],
    ])
    # bba

    dfa8 = dfa6.union(dfa7)
    # b((ab)|(ba))

    assert(dfa8("ab") == False)
    assert(dfa8("ba") == False)
    assert(dfa8("bab") == True)
    assert(dfa8("bba") == True)
    assert(dfa8("baa") == False)
    assert(dfa8("bbb") == False)


def test_match(pattern, string, expected):
    out = RegEx.match(pattern, string)
    if out != expected:
        raise Exception(f"outcome ({out}) is not expected ({expected})")


def test_regex():
    dfa = RegEx.DFA.from_regex("b(a|b)*b")
    assert(dfa("") == False)
    assert(dfa("b") == False)
    assert(dfa("bb") == True)
    assert(dfa("bab") == True)
    assert(dfa("ba") == False)
    assert(dfa("bab") == True)
    assert(dfa("bababbaab") == True)

    # empty string
    e = RegEx.DFA.from_regex("")
    assert(e("") == True)
    assert(e("a") == False)
    # "()" can specify empty string too
    e2 = RegEx.DFA.from_regex("()")
    assert(e2("") == True)
    assert(e2("a") == False)

    # emulate '?' operator
    # though I could, I choose not to make "(|abc)" legal
    zero_or_one = RegEx.DFA.from_regex("(()|abc)")
    assert(zero_or_one("") == True)
    assert(zero_or_one("abc") == True)
    assert(zero_or_one("abcabc") == False)

    # binary divisible by 3
    # https://stackoverflow.com/a/19608040/10899376
    div3 = RegEx.DFA.from_regex("(1(01*0)*1|0)*")
    for i in range(100):
        assert(div3(bin(i)[2:]) == (i % 3 == 0))


def main():
    print("testing basic")
    test_basic()
    print("testing NFA to DFA conversion")
    test_nfa_to_dfa()
    print("testing operation kleene star")
    test_kleene_star()
    print("testing operation concatenation")
    test_concat()
    print("testing operation union")
    test_union()
    print("testing regular expression to DFA conversion")
    test_regex()

    print("all passed")


if __name__ == "__main__":
    main()
