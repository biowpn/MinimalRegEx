# Minimal Regular Expression Engine

Python implementation of a minimal regular expression engine.

## Usage

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

## Testing

run [test.py](./test.py)

## Credits

Algorithms inspired by *[Elements of the Theory of Computation](https://dl.acm.org/citation.cfm?id=549820)* (2nd Edition) by Harry R. Lewis and Christos H. Papadimitriou.
