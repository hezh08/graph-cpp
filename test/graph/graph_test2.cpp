#include "gdwg/graph.hpp"

#include <catch2/catch.hpp>
#include <sstream>

// Rationale: test/README.md

// Modifiers

namespace helper {
	template<typename N, typename E>
	auto check_output_is_expected(gdwg::graph<N, E>& g, std::string_view const& expected_output) -> void {
		auto out = std::ostringstream{};
		out << g;
		CHECK(out.str() == expected_output);
	}
}

using namespace helper;

TEST_CASE("Test node insertion") {
	auto g = gdwg::graph<int, std::string>();

	SECTION("Check successful insertion") {
		REQUIRE(g.insert_node(5));
		CHECK(g.is_node(5));
		check_output_is_expected(g,
		                         std::string_view(
		                            R"(5 (
)
)"));
	}

	SECTION("Check repeated insertion returns false") {
		REQUIRE(g.insert_node(3));
		CHECK_FALSE(g.insert_node(3));
	}
}

TEST_CASE("Test edge insertion") {
	auto g = gdwg::graph<int, int>{1, 2, 3, 4, 5, 6};

	SECTION("Test functionality is correct for normal edge insertion") {
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
		for (const auto& [from, to, weight] : v) {
			REQUIRE(g.insert_edge(from, to, weight));
		};

		check_output_is_expected(g,
		                         std::string_view(
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
)"));
	}

	SECTION("Check repeated insertion returns false") {
		REQUIRE(g.insert_edge(6, 6, 6));
		CHECK_FALSE(g.insert_edge(6, 6, 6));
	}

	SECTION("Check exception is thrown if nodes do not exist") {
		REQUIRE_THROWS_MATCHES(g.insert_edge(1, 7, 7),
		                       std::runtime_error,
		                       Catch::Message("Cannot call gdwg::graph<N, E>::insert_edge when "
		                                      "either src or dst node does not exist"));
		REQUIRE_THROWS_MATCHES(g.insert_edge(8, 1, 8),
		                       std::runtime_error,
		                       Catch::Message("Cannot call gdwg::graph<N, E>::insert_edge when "
		                                      "either src or dst node does not exist"));
	}
}

TEST_CASE("Test node replacement") {
	auto g = gdwg::graph<int, std::string>{1, 2, 3, 4};
	g.insert_edge(1, 2, "Hello!");
	g.insert_edge(1, 3, "How");
	g.insert_edge(2, 3, "are");
	g.insert_edge(3, 3, "you?");

	SECTION("Check node is successfully replaced and edges are updated") {
		REQUIRE(g.replace_node(3, 6)); // Check node with edges
		REQUIRE(g.replace_node(4, 5)); // Check node with no edges
		CHECK_FALSE(g.is_node(3));
		CHECK_FALSE(g.is_node(4));
		check_output_is_expected(g,
		                         std::string_view(
		                            R"(1 (
  2 | Hello!
  6 | How
)
2 (
  6 | are
)
5 (
)
6 (
  6 | you?
)
)"));
	}

	SECTION("Check node cannot replace itself") {
		CHECK_FALSE(g.replace_node(3, 3));
		check_output_is_expected(g,
		                         std::string_view(
		                            R"(1 (
  2 | Hello!
  3 | How
)
2 (
  3 | are
)
3 (
  3 | you?
)
4 (
)
)"));
	}

	SECTION("Check function has no effect if replacement node already exists") {
		CHECK_FALSE(g.replace_node(3, 4));
	}

	SECTION("Check exception is thrown if node to be replaced does not exist") {
		REQUIRE_THROWS_MATCHES(g.replace_node(7, 8),
		                       std::runtime_error,
		                       Catch::Message("Cannot call gdwg::graph<N, E>::replace_node on a node "
		                                      "that doesn't exist"));
	}
}

TEST_CASE("Test node merger and replacement") {
	auto g = gdwg::graph<char, int>{'A', 'B', 'C', 'D'};

	SECTION("Check node is successfully merged and replaced - incoming edges") {
		g.insert_edge('A', 'B', 3);
		g.insert_edge('C', 'B', 2);
		g.insert_edge('D', 'B', 4);
		g.insert_edge('B', 'B', 5); // Check self-pointing
		g.merge_replace_node('B', 'A');

		CHECK_FALSE(g.is_node('B'));
		check_output_is_expected(g,
		                         std::string_view(
		                            R"(A (
  A | 3
  A | 5
)
C (
  A | 2
)
D (
  A | 4
)
)"));
	}

	SECTION("Check node is successfully merged and replaced - outgoing edges") {
		g.insert_edge('B', 'A', 3);
		g.insert_edge('B', 'C', 2);
		g.insert_edge('B', 'D', 4);
		g.insert_edge('B', 'B', 5); // Check self-pointing
		g.merge_replace_node('B', 'A');

		CHECK_FALSE(g.is_node('B'));
		check_output_is_expected(g,
		                         std::string_view(
		                            R"(A (
  A | 3
  A | 5
  C | 2
  D | 4
)
C (
)
D (
)
)"));
	}

	SECTION("Check for graphs with no edges") {
		g.merge_replace_node('B', 'A');
		check_output_is_expected(g,
		                         std::string_view(
		                            R"(A (
)
C (
)
D (
)
)"));
	}

	SECTION("Check node merger and replacement of itself has no effect") {
		g.insert_edge('B', 'A', 3);
		g.insert_edge('B', 'C', 2);
		g.insert_edge('B', 'D', 4);
		auto const g2 = g;
		REQUIRE_NOTHROW(g.merge_replace_node('B', 'B'));
		CHECK(g2 == g);
	}

	SECTION("Check exception is thrown if nodes do not exist") {
		REQUIRE_THROWS_MATCHES(g.merge_replace_node('A', 'E'),
		                       std::runtime_error,
		                       Catch::Message("Cannot call gdwg::graph<N, E>::merge_replace_node on "
		                                      "old or new data if they don't exist in the graph"));
		REQUIRE_THROWS_MATCHES(g.merge_replace_node('E', 'A'),
		                       std::runtime_error,
		                       Catch::Message("Cannot call gdwg::graph<N, E>::merge_replace_node on "
		                                      "old or new data if they don't exist in the graph"));
	}
}

TEST_CASE("Test node erasure") {
	auto g = gdwg::graph<double, int>{1.53, 325, 643.6, 99.99};
	g.insert_edge(1.53, 325, 1);
	g.insert_edge(643.6, 325, 2);
	g.insert_edge(325, 325, 3);

	SECTION("Check erasure on non-existent node") {
		CHECK_FALSE(g.erase_node(404));
	}

	SECTION("Check erasure on existing node") {
		CHECK(g.erase_node(325)); // Check node with edges
		CHECK(g.erase_node(99.99)); // Check node with no edges
		CHECK_FALSE(g.is_node(325));
		CHECK_FALSE(g.is_node(99.99));
		check_output_is_expected(g,
		                         std::string_view(
		                            R"(1.53 (
)
643.6 (
)
)"));
	}
}

TEST_CASE("Test edge erasure") {
	auto g = gdwg::graph<double, int>{1.53, 325, 643.6};
	g.insert_edge(1.53, 325, 1);
	g.insert_edge(643.6, 325, 2);
	g.insert_edge(325, 325, 3); // Check reflexive edge

	SECTION("Check erasure of non-existent weight") {
		CHECK_FALSE(g.erase_edge(1.53, 325, 404));
	}

	SECTION("Check successfully edge erasure") {
		CHECK(g.erase_edge(643.6, 325, 2));
		check_output_is_expected(g,
		                         std::string_view(
		                            R"(1.53 (
  325 | 1
)
325 (
  325 | 3
)
643.6 (
)
)"));
		CHECK(g.erase_edge(325, 325, 3));
		check_output_is_expected(g,
		                         std::string_view(
		                            R"(1.53 (
  325 | 1
)
325 (
)
643.6 (
)
)"));
	}

	SECTION("Check exceptions are thrown if nodes do not exist") {
		REQUIRE_THROWS_MATCHES(g.erase_edge(1.53, 404, 1),
		                       std::runtime_error,
		                       Catch::Message("Cannot call gdwg::graph<N, E>::erase_edge on src or "
		                                      "dst if they don't exist in the graph"));
		REQUIRE_THROWS_MATCHES(g.erase_edge(404, 1.53, 2),
		                       std::runtime_error,
		                       Catch::Message("Cannot call gdwg::graph<N, E>::erase_edge on src or "
		                                      "dst if they don't exist in the graph"));
	}
}

TEST_CASE("Test edge erasure by iterator") {
	SECTION("Check functionality is correct for graph that has edges") {
		auto g = gdwg::graph<char, int>{'A', 'B', 'C', 'D'};
		g.insert_edge('A', 'D', 1);
		g.insert_edge('B', 'A', 3);
		g.insert_edge('C', 'C', 2);
		g.insert_edge('B', 'D', 4);

		auto it = g.begin();
		REQUIRE_NOTHROW(it = g.erase_edge(it));
		CHECK((*it).from == 'B');
		CHECK((*it).to == 'A');
		CHECK((*it).weight == 3);
		REQUIRE_NOTHROW(it = g.erase_edge(it));
		CHECK((*it).from == 'B');
		CHECK((*it).to == 'D');
		CHECK((*it).weight == 4);
		REQUIRE_NOTHROW(it = g.erase_edge(it));
		CHECK((*it).from == 'C');
		CHECK((*it).to == 'C');
		CHECK((*it).weight == 2);
		REQUIRE_NOTHROW(it = g.erase_edge(it));
		CHECK(it == g.end());
	}

	SECTION("Check erasure on graph with no edges") {
		auto g = gdwg::graph<int, int>{1, 2, 3, 4};
		auto it = g.begin();
		REQUIRE_NOTHROW(g.erase_edge(it));
		CHECK(it == g.end());
	}

	SECTION("Check erasure on empty graph") {
		auto g = gdwg::graph<int, int>();
		auto it = g.begin();
		REQUIRE_NOTHROW(g.erase_edge(it));
		CHECK(it == g.end());
	}
}

TEST_CASE("Test edge erasure of range of iterators") {
	auto g = gdwg::graph<char, int>{'A', 'B', 'C', 'D'};
	g.insert_edge('A', 'D', 1);
	g.insert_edge('B', 'A', 3);
	g.insert_edge('C', 'C', 2);
	g.insert_edge('B', 'D', 4);

	SECTION("Test edges are erased from lower range up to but not including upper range") {
		// i.e. test [i, s) erasure
		auto it = g.begin();
		REQUIRE_NOTHROW(it = g.erase_edge((++g.begin()), (--g.end())));
		CHECK((*it).from == 'C');
		CHECK((*it).to == 'C');
		CHECK((*it).weight == 2);
		check_output_is_expected(g,
		                         std::string_view(
		                            R"(A (
  D | 1
)
B (
)
C (
  C | 2
)
D (
)
)"));
	}

	SECTION("Check edges are cleared if range is from begin() to end()") {
		REQUIRE_NOTHROW(g.erase_edge(g.begin(), g.end()));
		check_output_is_expected(g,
		                         std::string_view(
		                            R"(A (
)
B (
)
C (
)
D (
)
)"));
	}

	SECTION("Check graph is unchanged if range iterators are the same") {
		REQUIRE_NOTHROW(g.erase_edge(g.begin(), g.begin()));
		check_output_is_expected(g,
		                         std::string_view(
		                            R"(A (
  D | 1
)
B (
  A | 3
  D | 4
)
C (
  C | 2
)
D (
)
)"));
	}
}

TEST_CASE("Test graph clearance") {
	auto g = gdwg::graph<char, int>{'A', 'B', 'C', 'D'};
	g.insert_edge('B', 'A', 3);
	g.insert_edge('B', 'C', 2);
	g.insert_edge('B', 'D', 4);

	SECTION("Test graph is cleared and no exception is thrown") {
		REQUIRE_NOTHROW(g.clear());
		REQUIRE(g.empty());
	}
}