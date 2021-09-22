#include "gdwg/graph.hpp"

#include <catch2/catch.hpp>
#include <sstream>

// Rationale: test/README.md

// Comparisons and Extractor

TEST_CASE("Test operator== and operator!= comparisons work for graphs") {
	SECTION("Check for graph with edges") {
		auto g = gdwg::graph<int, std::string>{1, 2, 3, 4};
		g.insert_edge(1, 2, "Hello!");
		g.insert_edge(1, 3, "How");
		g.insert_edge(2, 3, "are");
		g.insert_edge(4, 3, "you?");
		auto g2 = gdwg::graph<int, std::string>{1, 2, 3, 4};
		g2.insert_edge(1, 3, "How");
		g2.insert_edge(2, 3, "are");
		g2.insert_edge(1, 2, "Hello!");
		g2.insert_edge(4, 3, "you?");
		auto g3 = gdwg::graph<int, std::string>{1, 2, 3, 4};
		g3.insert_edge(1, 3, "How");
		g3.insert_edge(2, 3, "are");
		g3.insert_edge(1, 2, "H3llo!");
		g3.insert_edge(4, 3, "you?");

		CHECK(g == g2);
		CHECK_FALSE(g == g3);
		CHECK(g != g3);
		CHECK_FALSE(g != g2);
	}

	SECTION("Check for graph with no edges") {
		auto const& g = gdwg::graph<int, std::string>{1, 2, 3, 4};
		auto const& g2 = gdwg::graph<int, std::string>{1, 2, 3, 4};
		auto const& g3 = gdwg::graph<int, std::string>{1, 2, 3, 8};

		CHECK(g == g2);
		CHECK_FALSE(g == g3);
		CHECK(g != g3);
		CHECK_FALSE(g != g2);
	}

	SECTION("Check for empty graphs") {
		auto const& g = gdwg::graph<int, std::string>{};
		auto const& g2 = gdwg::graph<int, std::string>{};
		CHECK(g == g2);
		CHECK_FALSE(g != g2);
	}
}

TEST_CASE("Test extractor prints correct output") {
	SECTION("Check output for non-empty graphs") {
		auto const v = std::vector<gdwg::graph<int, int>::value_type>{
		   {4, 1, -4},
		   {3, 2, 2},
		   {2, 4, 2},
		   {2, 1, 1},
		   {6, 2, 5},
		   {6, 3, 10},
		   {1, 5, -1},
		   {3, 6, -8},
		   {4, 5, 3},
		   {5, 2, 7},
		};

		auto g = gdwg::graph<int, int>{1, 2, 3, 4, 5, 6};
		for (const auto& [from, to, weight] : v) {
			g.insert_edge(from, to, weight); // Check nodes with edges
		};

		g.insert_node(64); // Check nodes with no edges
		auto out = std::ostringstream{};
		out << g;
		auto const expected_output = std::string_view(
		   R"(1 (
  5 | -1
)
2 (
  1 | 1
  4 | 2
)
3 (
  2 | 2
  6 | -8
)
4 (
  1 | -4
  5 | 3
)
5 (
  2 | 7
)
6 (
  2 | 5
  3 | 10
)
64 (
)
)");
		CHECK(out.str() == expected_output);
	}

	SECTION("Check for empty graphs") {
		auto g = gdwg::graph<int, int>{};
		auto out = std::ostringstream{};
		out << g;
		auto const expected_output = std::string_view(R"()");
		CHECK(out.str() == expected_output);
	}
}