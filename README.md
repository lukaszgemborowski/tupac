# tupac
Ever wanted to easily edit a tuple of values in C++? Like:

```
auto tup = make_tuple(...);
auto tup2 = tup | remove_if(is_class) | push_back(42);
```

the library is just for this purpose!
