# Rationale

### 1

Test files 1-5 are divided by the sections on the specification [README.md](../README.md):
* [Test 1 - Constructors](./graph/graph_test1.cpp)
* [Test 2 - Modifiers](./graph/graph_test2.cpp)
* [Test 3 - Accessors](./graph/graph_test3.cpp)
* [Test 4 - Iterator Access and Iterator](./graph/graph_test4.cpp)
* [Test 5 - Comparisons and Extractor](./graph/graph_test5.cpp)

Every function in each section has its own `TEST_CASE`.

***

### 2

Where applicable I divide my testing to check:
1. empty graphs,
2. node-only graphs, and
3. graph with edges.

I consider these type of graphs to be edge cases with potentially unexpected results.

I also check reflexive edges if applicable, particularly in 'Test 2 - Modifiers'.

***

### 3

For Test 1 and Test 2, I use a helper function called `check_output_is_expected()`.

In the test cases, the graphs' nodes and edges are checked to be correct by comparing the output to a raw string literal.   That is, I use `out << g` and `out.str == expected`.

This is an alternative to using `g.is_node(node)` and `g.find(edge)`.

While the maintainability of the test code decreases, since you will have to modify the test cases if the graph's output format changes, I believe that a output string comparison is better for test accuracy.

This is because the graph output exposes all nodes and edges on a graph, while `g.find(edge)` and `g.is_node(node)` will only find specific nodes and edges. It will not show any hidden or unexpected nodes and edges. Thus, string comparison is used.

***

### 4

For exception-throwing functions, such as in 'Test 2 - Modifiers', I check that exceptions are thrown with the correct message. I consider the exception message part of the functionality, hence it is checked as well.

***

### 5

Functions marked 'const', particularly those in 'Test 3 - Accessors' are tested using const variables or const containers to ensure that the function maintains constness.

***

### 6

`REQUIRE` is used, as opposed to `CHECK`, when a condition must be fulfilled before checking any subsequent conditions.

For example, the following code requires the first function to be completed successfully in order to check for repetitions:
```
SECTION("Check repeated insertion returns false") {
    REQUIRE(g.insert_edge(6, 6, 6));
    CHECK_FALSE(g.insert_edge(6, 6, 6));
}
```

***

### 7

Most tests are independent of each other, or rely on previously tested functions.

Some tests borrow functions such as `g.begin()` or `out << g` or `g.find(edge)`. These are unavoidable.