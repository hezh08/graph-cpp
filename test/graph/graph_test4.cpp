#include "gdwg/graph.hpp"

#include <catch2/catch.hpp>

// Rationale: test/README.md

// Iterator Access and Iterator

TEST_CASE("Test iterator access - begin() and end()") {
	SECTION("Check that begin() is same as end() for graphs with no edges") {
		auto g = gdwg::graph<int, int>{1, 2, 3};
		CHECK(g.begin() == g.end());
	}

	SECTION("Check that begin() is same as end() for empty graphs") {
		auto g = gdwg::graph<char, int>{};
		CHECK(g.begin() == g.end());
	}

	SECTION("Check that begin() points to first edge in a graph which is lexigraphically sorted") {
		auto g = gdwg::graph<int, std::string>{3, 2, 1};
		g.insert_edge(2, 3, "ABCDEF");
		g.insert_edge(2, 3, "ABCDEE");

		auto const it = g.begin();
		CHECK((*it).from == 2);
		CHECK((*it).to == 3);
		CHECK((*it).weight == "ABCDEE");
	}
}

TEST_CASE("Test iterator type is public") {
	auto g = gdwg::graph<char, int>{'D', 'A', 'C', 'B'};
	g.insert_edge('A', 'B', 6);
	g.insert_edge('A', 'B', 1);

	REQUIRE_NOTHROW(gdwg::graph<char, int>::iterator{});
}

TEST_CASE("Test iterator source points to underlying element") {
	auto g = gdwg::graph<char, int>{'D', 'A', 'C', 'B'};
	g.insert_edge('A', 'B', 6);
	g.insert_edge('A', 'B', 1);

	auto const it = g.begin();
	CHECK((*it).from == 'A');
	CHECK((*it).to == 'B');
	CHECK((*it).weight == 1);
}

TEST_CASE("Test iterator prefix and postfix increments") {
	auto g = gdwg::graph<char, int>{'D', 'A', 'C', 'B'};
	g.insert_edge('C', 'D', 8);
	g.insert_edge('A', 'B', 6);
	g.insert_edge('A', 'B', 1);

	SECTION("Check prefix increment") {
		auto it = g.begin();
		++it;
		CHECK((*it).from == 'A');
		CHECK((*it).to == 'B');
		CHECK((*it).weight == 6);
		++it;
		CHECK((*it).from == 'C');
		CHECK((*it).to == 'D');
		CHECK((*it).weight == 8);
		++it;
		CHECK(it == g.end());
	}

	SECTION("Check postfix increment") {
		auto it = g.begin();
		it++;
		CHECK((*it).from == 'A');
		CHECK((*it).to == 'B');
		CHECK((*it).weight == 6);
		it++;
		CHECK((*it).from == 'C');
		CHECK((*it).to == 'D');
		CHECK((*it).weight == 8);
		it++;
		CHECK(it == g.end());
	}
}

TEST_CASE("Test iterator prefix and postfix decrements") {
	auto g = gdwg::graph<char, int>{'D', 'A', 'C', 'B'};
	g.insert_edge('C', 'D', 8);
	g.insert_edge('A', 'B', 6);
	g.insert_edge('A', 'B', 1);

	SECTION("Check prefix decrement") {
		auto it = g.end();
		--it;
		CHECK((*it).from == 'C');
		CHECK((*it).to == 'D');
		CHECK((*it).weight == 8);
		--it;
		CHECK((*it).from == 'A');
		CHECK((*it).to == 'B');
		CHECK((*it).weight == 6);
		--it;
		CHECK(it == g.begin());
	}

	SECTION("Check postfix decrement") {
		auto it = g.end();
		it--;
		CHECK((*it).from == 'C');
		CHECK((*it).to == 'D');
		CHECK((*it).weight == 8);
		it--;
		CHECK((*it).from == 'A');
		CHECK((*it).to == 'B');
		CHECK((*it).weight == 6);
		it--;
		CHECK(it == g.begin());
	}
}

TEST_CASE("Test matching iterators are compared equal") {
	SECTION("Check iterators of the same graph can be equal") {
		auto g = gdwg::graph<char, int>{'D', 'A', 'C', 'B'};
		g.insert_edge('C', 'D', 8);

		auto it1 = g.begin();
		auto it2 = (--g.end());
		auto it3 = it1;

		CHECK(it1 == it2);
		CHECK(it2 == it3);
	}

	SECTION("Check iterators of different graphs are not equal") {
		auto g = gdwg::graph<char, int>{'A', 'M', 'B', 'C'};
		auto g2 = gdwg::graph<char, int>{'A', 'M', 'B', 'E'};
		g.insert_edge('M', 'B', 8);
		g2.insert_edge('M', 'B', 8);

		auto it1 = g.begin();
		auto it2 = g2.begin();

		CHECK_FALSE(it1 == it2);
	}
}

TEST_CASE("Test iterator traversal is in lexigraphically sorted order") {
	auto const v = std::vector<gdwg::graph<int, int>::value_type>{
	   {21, 14, 23},
	   {21, 31, 14},
	   {14, 14, 0},
	   {12, 19, 16},
	   {19, 21, 2},
	   {19, 1, 3},
	   {1, 12, 3},
	   {1, 7, 4},
	   {1, 21, 12},
	   {7, 21, 13},
	};

	auto g = gdwg::graph<int, int>{67, 31, 12, 19, 21, 7, 1, 14};
	for (const auto& [from, to, weight] : v) {
		g.insert_edge(from, to, weight);
	}

	auto const sorted = std::vector<gdwg::graph<int, int>::value_type>{
	   {1, 7, 4},
	   {1, 12, 3},
	   {1, 21, 12},
	   {7, 21, 13},
	   {12, 19, 16},
	   {14, 14, 0},
	   {19, 1, 3},
	   {19, 21, 2},
	   {21, 14, 23},
	   {21, 31, 14},
	};

	CHECK(std::equal(g.begin(), g.end(), sorted.begin(), [](auto const& lhs, auto const& rhs) {
		return (lhs.from == rhs.from and lhs.to == rhs.to and lhs.weight == rhs.weight);
	}));
}