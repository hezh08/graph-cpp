#include "gdwg/graph.hpp"

#include <catch2/catch.hpp>
#include <sstream>

// Rationale: test/README.md

// Constructors

namespace helper {
	template<typename N, typename E>
	auto check_output_is_expected(gdwg::graph<N, E>& g, std::string_view const& expected_output) -> void {
		auto out = std::ostringstream{};
		out << g;
		CHECK(out.str() == expected_output);
	}
}

using namespace helper;

TEST_CASE("Test basic constructor") {
	auto g = gdwg::graph<int, std::string>();
	CHECK(g.empty());
	CHECK_NOTHROW(gdwg::graph<int, std::string>{});
}

TEST_CASE("Test constructor using initializer list") {
	auto g = gdwg::graph<int, std::string>{1, 2, 3, 4};
	CHECK(g.is_node(1));
	CHECK(g.is_node(2));
	CHECK(g.is_node(3));
	CHECK(g.is_node(4));
}

TEST_CASE("Test constructor using input iterators") {
	auto v = std::vector<int>{1, 2, 3, 4};
	auto g = gdwg::graph<int, std::string>(v.begin(), v.end());
	CHECK(g.is_node(1));
	CHECK(g.is_node(2));
	CHECK(g.is_node(3));
	CHECK(g.is_node(4));
}

TEST_CASE("Test move constructor") {
	auto g = gdwg::graph<int, std::string>{1, 2, 3, 4};
	g.insert_edge(1, 2, "Hello!");
	g.insert_edge(1, 3, "How");
	g.insert_edge(2, 3, "are");
	g.insert_edge(4, 3, "you?");

	SECTION("Check no exception is thrown") {
		REQUIRE_NOTHROW(gdwg::graph<int, std::string>(std::move(g)));
	}

	SECTION("Check graph contents are successfully moved") {
		auto it = g.begin();
		auto g2 = std::move(g);
		CHECK(g2.is_node(1));
		CHECK(g2.is_node(2));
		CHECK(g2.is_node(3));
		CHECK(g2.is_node(4));
		check_output_is_expected(g2,
		                         std::string_view(
		                            R"(1 (
  2 | Hello!
  3 | How
)
2 (
  3 | are
)
3 (
)
4 (
  3 | you?
)
)"));

		CHECK(g.empty());

		++it; // Check iterator points to moved graph
		CHECK((*it).from == 1);
		CHECK((*it).to == 3);
		CHECK((*it).weight == "How");
	}
}

TEST_CASE("Test copy constructor") {
	auto g = gdwg::graph<int, std::string>{1, 2, 3, 4};
	g.insert_edge(1, 2, "Hello!");
	g.insert_edge(1, 3, "How");
	g.insert_edge(2, 3, "are");
	g.insert_edge(4, 3, "you?");

	auto g2 = g;
	CHECK(g2.is_node(1));
	CHECK(g2.is_node(2));
	CHECK(g2.is_node(3));
	CHECK(g2.is_node(4));
	check_output_is_expected(g2,
	                         std::string_view(
	                            R"(1 (
  2 | Hello!
  3 | How
)
2 (
  3 | are
)
3 (
)
4 (
  3 | you?
)
)"));
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
)
4 (
  3 | you?
)
)"));
}

TEST_CASE("Test move assignment") {
	auto g = gdwg::graph<int, std::string>{1, 2, 3, 4};
	g.insert_edge(1, 2, "Hello!");
	g.insert_edge(1, 3, "How");
	g.insert_edge(2, 3, "are");
	g.insert_edge(4, 3, "you?");

	SECTION("Check no exception is thrown") {
		auto g2 = gdwg::graph<int, std::string>();
		REQUIRE_NOTHROW(g2 = std::move(g));
	}

	SECTION("Check graph contents are successfully moved") {
		auto it = g.begin();
		auto g2 = gdwg::graph<int, std::string>{};
		g2 = std::move(g);
		CHECK(g2.is_node(1));
		CHECK(g2.is_node(2));
		CHECK(g2.is_node(3));
		CHECK(g2.is_node(4));
		check_output_is_expected(g2,
		                         std::string_view(
		                            R"(1 (
  2 | Hello!
  3 | How
)
2 (
  3 | are
)
3 (
)
4 (
  3 | you?
)
)"));
		CHECK(g.empty());

		++it; // Check iterator points to moved graph
		CHECK((*it).from == 1);
		CHECK((*it).to == 3);
		CHECK((*it).weight == "How");
	}
}

TEST_CASE("Test copy assignment") {
	auto g = gdwg::graph<int, std::string>{1, 2, 3, 4};
	auto g2 = gdwg::graph<int, std::string>{};
	g.insert_edge(1, 2, "Hello!");
	g.insert_edge(1, 3, "How");
	g.insert_edge(2, 3, "are");
	g.insert_edge(4, 3, "you?");

	g2 = g;
	CHECK(g2.is_node(1));
	CHECK(g2.is_node(2));
	CHECK(g2.is_node(3));
	CHECK(g2.is_node(4));

	check_output_is_expected(g2,
	                         std::string_view(
	                            R"(1 (
  2 | Hello!
  3 | How
)
2 (
  3 | are
)
3 (
)
4 (
  3 | you?
)
)"));
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
)
4 (
  3 | you?
)
)"));
}