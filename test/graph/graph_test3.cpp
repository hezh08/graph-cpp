#include "gdwg/graph.hpp"

#include <catch2/catch.hpp>

// Rationale: test/README.md

// Accessors

TEST_CASE("Test is_node() identifies nodes existing in graph") {
	auto const g = gdwg::graph<char, int>{'A'};

	CHECK(g.is_node('A'));
	CHECK_FALSE(g.is_node('B'));
}

TEST_CASE("Test empty() identifies empty graphs") {
	auto const filled = gdwg::graph<char, int>{'A', 'B', 'C', 'D'};
	auto const empty = gdwg::graph<char, int>{};

	CHECK_FALSE(filled.empty());
	CHECK(empty.empty());
}

TEST_CASE("Test is_connected() identifies connected nodes in graph") {
	auto g = gdwg::graph<char, int>{'A', 'B', 'C', 'D'};
	g.insert_edge('A', 'B', 3);

	SECTION("Check connection is identified in correct direction of a directed edge") {
		auto const g2 = g;
		CHECK(g2.is_connected('A', 'B'));
		CHECK_FALSE(g.is_connected('B', 'A'));
	}

	SECTION("Check connection is present for reflexive nodes") {
		g.insert_edge('D', 'D', 6);
		CHECK(g.is_connected('D', 'D'));
	}

	SECTION("Check connection is not identified for separated nodes") {
		CHECK_FALSE(g.is_connected('A', 'D'));
		CHECK_FALSE(g.is_connected('C', 'D'));
	}

	SECTION("Check exception is thrown if nodes do not exist") {
		REQUIRE_THROWS_MATCHES(g.is_connected('A', 'E'),
		                       std::runtime_error,
		                       Catch::Message("Cannot call gdwg::graph<N, E>::is_connected if src or "
		                                      "dst node don't exist in the graph"));
		REQUIRE_THROWS_MATCHES(g.is_connected('E', 'A'),
		                       std::runtime_error,
		                       Catch::Message("Cannot call gdwg::graph<N, E>::is_connected if src or "
		                                      "dst node don't exist in the graph"));
	}
}

TEST_CASE("Test nodes() stores a sequence of all nodes") {
	SECTION("Check nodes() is empty if graph has no nodes") {
		auto const g = gdwg::graph<double, int>{};
		auto v = g.nodes();
		CHECK(v.empty());
	}

	SECTION("Check nodes() stores correctly and is sorted") {
		auto g = gdwg::graph<int, int>{4, 3, 2};
		g.insert_node(1);
		g.insert_node(5);
		auto v = g.nodes();
		auto expected = std::vector<int>{1, 2, 3, 4, 5};
		CHECK(v == expected);
		CHECK(std::is_sorted(v.begin(), v.end()));
	}
}

TEST_CASE("Test weights() returns all weights between two nodes") {
	auto g = gdwg::graph<std::string, int>{"Hello", "How", "are", "you?"};
	g.insert_edge("Hello", "are", 3);
	g.insert_edge("Hello", "are", 1);
	g.insert_edge("Hello", "are", 4);
	g.insert_edge("Hello", "are", 2);

	SECTION("Check weights() is empty for separated nodes") {
		auto v = g.weights("you?", "are");
		CHECK(v.empty());
	}

	SECTION("Check weights() stores correctly and is sorted") {
		auto const g2 = g;
		auto v = g2.weights("Hello", "are");
		auto expected = std::vector<int>{1, 2, 3, 4};
		CHECK(v == expected);
		CHECK(std::is_sorted(v.begin(), v.end()));
	}

	SECTION("Check exception is thrown if nodes do not exist") {
		REQUIRE_THROWS_MATCHES(g.weights("Hello", "Howdy"),
		                       std::runtime_error,
		                       Catch::Message("Cannot call gdwg::graph<N, E>::weights if src or dst "
		                                      "node don't exist in the graph"));
		REQUIRE_THROWS_MATCHES(g.weights("Aloha", "are"),
		                       std::runtime_error,
		                       Catch::Message("Cannot call gdwg::graph<N, E>::weights if src or dst "
		                                      "node don't exist in the graph"));
	}
}

TEST_CASE("Test find() finds edges") {
	auto g = gdwg::graph<std::string, int>{"Hello", "How", "are", "you?"};
	g.insert_edge("Hello", "are", 3);
	g.insert_edge("Hello", "are", 1);
	g.insert_edge("Hello", "are", 4);
	g.insert_edge("Hello", "are", 2);

	SECTION("Check find() returns an iterator to the correct edge") {
		auto const g2 = g;
		auto it = g2.find("Hello", "are", 4);
		CHECK((*it).from == "Hello");
		CHECK((*it).to == "are");
		CHECK((*it).weight == 4);
	}

	SECTION("Check find() returns end() if edge does not exist") {
		CHECK(g.find("Hello", "are", 5) == g.end());
		CHECK(g.find("you?", "are", 4) == g.end());
		CHECK(g.find("Hello", "How", 4) == g.end());
	}
}

TEST_CASE("Test connections() gets all outgoing edges of a source node") {
	auto g = gdwg::graph<std::string, int>{"Hello", "How", "are", "you?"};
	g.insert_edge("Hello", "How", 3);
	g.insert_edge("Hello", "are", 1);
	g.insert_edge("Hello", "you?", 4);
	g.insert_edge("Hello", "Hello", 2);

	SECTION("Check functionality returns all outgoing edges") {
		auto const g2 = g;
		auto v = g2.connections("Hello");
		auto expected = std::vector<std::string>{"Hello", "How", "are", "you?"};
		CHECK(v == expected);
		CHECK(std::is_sorted(v.begin(), v.end()));
	}

	SECTION("Check zero connections are returned for nodes with no outgoing edges") {
		auto v = g.connections("are");
		CHECK(v.empty());
	}

	SECTION("Check exception is thrown if node does not exist") {
		REQUIRE_THROWS_MATCHES(g.connections("Howdy"),
		                       std::runtime_error,
		                       Catch::Message("Cannot call gdwg::graph<N, E>::connections if src "
		                                      "doesn't exist in the graph"));
	}
}