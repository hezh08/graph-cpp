# Directed Weighted Graph Template Library - C++

This is a project which I did during my C++ programming course COMP6771 at UNSW.

It is a header-only template library for directed weighted graphs. 

In COMP6771, we used the alias of generic directed weighted graph (gdwg).

## Usage

To use the library, include `include/gdwg/graph.hpp` in your source files.

To run tests, please install Catch2 test framework and run CMake.

Update - 22/09/2021 - New Catch2 downloads will be using Catch V3 libraries so you'll have to link the old Catch V2 libaries in order for the tests to work.

## Library constructors and methods



```cpp
// Constructors
graph();
graph(std::initializer_list<N>);
template<typename InputIt>
graph(InputIt, InputIt);
graph(graph const&);
graph(graph&&) noexcept;
auto operator=(graph const&) -> graph&;
auto operator=(graph&&) noexcept -> graph&;

// Modifiers
auto insert_node(N const&) -> bool;
auto insert_edge(N const&, N const&, E const&) -> bool;
auto replace_node(N const&, N const&) -> bool;
auto merge_replace_node(N const&, N const&) -> void;
auto erase_node(N const&) noexcept -> bool;
auto erase_edge(N const&, N const&, E const&) -> bool;
auto erase_edge(iterator) noexcept -> iterator;
auto erase_edge(iterator, iterator) noexcept -> iterator;
auto clear() noexcept -> void;

// Accessors
[[nodiscard]] auto is_node(N const&) const noexcept -> bool;
[[nodiscard]] auto empty() const noexcept -> bool;
[[nodiscard]] auto is_connected(N const&, N const&) const -> bool;
[[nodiscard]] auto nodes() const -> std::vector<N>;
[[nodiscard]] auto weights(N const&, N const&) const -> std::vector<E>;
[[nodiscard]] auto find(N const&, N const&, E const&) const -> iterator;
[[nodiscard]] auto connections(N const&) const -> std::vector<N>;

// Iterator access
[[nodiscard]] auto begin() const -> iterator;
[[nodiscard]] auto end() const -> iterator;

// Comparisons
[[nodiscard]] auto operator==(graph const&) const noexcept -> bool;

// Extractor
friend auto operator<<(std::ostream&, graph const&) -> std::ostream&;
```