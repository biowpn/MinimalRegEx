# Minimal Regular Expression Engine

Python and C++ implementation of a minimal regular expression engine (support only the three basic operations (concatenation, alternation, and kleene star) and parenthesis).

## Usage

- Python

    1. directly calling `match`:

        ```
        import RegEx

        RegEx.match("a(a|b|c)*b", "abacabb")
        ```

    2. construct DFA first, then eval against input:

        ```
        import RegEx

        dfa = RegEx.DFA.from_regex("a(a|b|c)*b")
        dfa("abacabb")
        ```
- C++

    1. directly calling `match`:

        ```
        #include "RegEx.h"

        RegEx::match("a(a|b|c)*b", "abacabb");
        ```

    2. construct DFA first, then eval against input:

        ```
        #include "RegEx.h"

        auto dfa = RegEx::DFA::from_regex("a(a|b|c)*b");
        dfa("abacabb");
        ```    

## Testing

- Python
    - run [test.py](./python/test.py)

- C++
    - `g++ --std=c++11 cpp/RegEx.cpp cpp/test.cpp -o test`
    - execute `test`

## Credits

Algorithms inspired by *[Elements of the Theory of Computation](https://dl.acm.org/citation.cfm?id=549820)* (2nd Edition) by Harry R. Lewis and Christos H. Papadimitriou.
