
A tiny C testing framework with opinions and very little patience.

`detest` is a small, header-only testing framework for C, inspired by
[greatest.h](https://github.com/silentbicycle/greatest), but intentionally minimal and explicit.

No runners.
No magic configuration.
No dependency hell.

Just C, macros, and test results that tell the truth.

## Features

- Single-header framework
- Automatic test registration (constructor-based)
- Suite and test filtering
- Assertion counting and timing
- Optional CTRF JSON report output
- Colored output (unless muted)
- Designed for low-level and embedded-style C projects

## Basic usage

Write your tests using `DETEST_TEST`:

```c
DETEST_TEST(math, addition)
{
  ASSERT_EQ(2 + 2, 4);
}
```
In one compilation unit, define the framework implementation and call `detest_run()`.

See main.c for a complete example.


Useful options:
```
-s <filter> : suite filter

-t <filter> : test filter

-l : list tests

-q : quiet mode

-j <file> : write CTRF JSON report
```

Exit codes

```
0 : all tests passed

>0 : number of failed tests
```

## Philosophy

If a test fails, it should say where and why.
If it segfaults, it should not pretend otherwise.

If you want mocks, fixtures and emotional support,
this is not your framework.

## License

Do whatever you want.
If it breaks, you get to keep both pieces.

