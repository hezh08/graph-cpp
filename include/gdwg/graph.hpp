#ifndef GDWG_GRAPH_HPP
#define GDWG_GRAPH_HPP

#include <algorithm>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <vector>

namespace gdwg {
	template<typename N, typename E>
	class graph {
		class iterator;

	public:
		struct value_type {
			N from;
			N to;
			E weight;
		};

		// Constructors
		graph() = default;
		
		graph(std::initializer_list<N> il) : graph(il.begin(), il.end()) {}

		template<typename InputIt>
		graph(InputIt first, InputIt last) : nodes_{}, repr_{} {
			std::for_each(first, last, [&](auto const& value) { insert_node(value); });
		}

		graph(graph&& other) noexcept 
		: nodes_{std::exchange(other.nodes_, {})}
		, repr_{std::exchange(other.repr_, {})} {}

		graph(graph const& other) : nodes_{other.nodes_}, repr_{other.repr_} {}

		auto operator=(graph&& other) noexcept -> graph& {
			std::swap(this->nodes_, other.nodes_);
			std::swap(this->repr_, other.repr_);
			return *this;
		}

		auto operator=(graph const& other) -> graph& {
			this->nodes_ = other.nodes_;
			this->repr_ = other.repr_;
			return *this;
		}

		// Modifiers

		auto insert_node(N const& value) -> bool {
			auto const& node_ptr = std::make_shared<N>(value);
			auto const& ret = nodes_.insert(node_ptr);
			if (ret.second) { // Only continues if node does not exist
				repr_.emplace(node_ptr.get(), std::set<std::pair<N*, E>, EdgeCompare>{});
				return true;
			}
			return false;
		}

		auto insert_edge(N const& src, N const& dst, E const& weight) -> bool {
			auto const& src_node = repr_.find(src);
			auto const& dst_node = repr_.find(dst);
			if (src_node != repr_.end() and dst_node != repr_.end()) {
				auto const& ret = src_node->second.emplace(dst_node->first, weight);
				return ret.second; // Returns true only if insertion took place
			}
			else {
				throw std::runtime_error("Cannot call gdwg::graph<N, E>::insert_edge when either src or "
										"dst node does not exist");
			}
		}

		auto replace_node(N const& old_data, N const& new_data) -> bool {
			// Check old node exists on graph, or otherwise throw an exception
			auto const& old_node = nodes_.find(old_data);
			if (old_node != nodes_.end()) {
				// Create the new node and get its raw pointer
				auto const& node_ptr = std::make_shared<N>(new_data);
				auto const& ret = nodes_.insert(node_ptr);
				if (not ret.second) { // If node already exists, it will not insert. Function then
									// discontinues.
					return false;
				}

				// Update the existing directed edges
				std::for_each(repr_.begin(), repr_.end(), [&](auto& key_value) {
					auto& edges = key_value.second;
					// For each old edge, extract it out of the set of edges,
					// change the destination node to the new node,
					// and reinsert it into the set of edges
					auto [begin, end] = edges.equal_range((*old_node).get());
					std::for_each(begin, end, [&](auto& edge) {
						auto tmp = edges.extract(edge);
						tmp.value().first = node_ptr.get();
						edges.insert(std::move(tmp));
					});
				});
				// Replace the directed edges outgoing from the old node
				auto tmp = repr_.extract((*old_node).get());
				tmp.key() = node_ptr.get();
				repr_.insert(std::move(tmp));

				nodes_.erase(old_node);
				return true;
			}
			else {
				throw std::runtime_error("Cannot call gdwg::graph<N, E>::replace_node on a node that "
										"doesn't exist");
			}
		}

		auto merge_replace_node(N const& old_data, N const& new_data) -> void {
			// Check both nodes exist on graph, otherwise throw an exception
			auto const& old_node = repr_.find(old_data);
			auto const& new_node = repr_.find(new_data);
			if (old_node != repr_.end() and new_node != repr_.end()) {
				if (old_node == new_node) {
					return; // Abort if nodes are the same
				}
				// Update the existing directed edges
				std::for_each(repr_.begin(), repr_.end(), [&](auto& key_value) {
					auto& edges = key_value.second;
					// For each old edge, extract it out of the set of edges,
					// change the destination node to the new node,
					// and reinsert it into the set of edges
					auto [begin, end] = edges.equal_range(old_node->first);
					std::for_each(begin, end, [&](auto& edge) {
						auto tmp = edges.extract(edge);
						tmp.value().first = new_node->first;
						edges.insert(std::move(tmp));
						// If edge does not insert (since it if it already exists),
						// edge will be removed by destructor of node handler at the end of the loop
					});
				});
				// Replace the directed edges outgoing from the old node.
				// Since key already exists, merge all edges to existing key
				// and delete key of old node
				new_node->second.merge(old_node->second);

				repr_.erase(old_node->first);
				nodes_.erase(nodes_.find(old_data));
			}
			else {
				throw std::runtime_error("Cannot call gdwg::graph<N, E>::merge_replace_node on old or new "
										"data if they don't exist in the graph");
			}
		}

		auto erase_node(N const& value) noexcept -> bool {
			auto const& node = nodes_.find(value);
			if (node != nodes_.end()) {
				// Erase all incoming edges
				for (auto& [src, edges] : repr_) {
					for (auto it = edges.find((*node).get()); it != edges.end(); ++it) {
						if (*(it->first) == **node) {
							it = edges.erase(it);
							if (it == edges.end()) {
								break;
							}
						}
						else {
							break;
						}
					}
				}
				// Erase all outgoing edges and the node
				repr_.erase((*node).get());
				nodes_.erase(node);

				return true;
			}
			return false;
		}

		auto erase_edge(N const& src, N const& dst, E const& weight) -> bool {
			// Time complexity
			//        checking nodes exist     - 2 log(n)
			//        erasing pair             - log(e) + n_keys
			//    = O(log(n) + log(e))
			//  < O(log(n) + e) solution
			auto const& src_node = repr_.find(src);
			auto const& dst_node = repr_.find(dst);
			if (src_node != repr_.end() and dst_node != repr_.end()) {
				auto const& ret = src_node->second.erase(std::make_pair(dst_node->first, weight));
				return (ret == 1); // Return true if edge is successfully erased
			}
			else {
				throw std::runtime_error("Cannot call gdwg::graph<N, E>::erase_edge on src or dst if they "
										"don't exist in the graph");
			}
		}

		auto erase_edge(iterator i) noexcept -> iterator {
			if (i == end())
				return i;

			auto copy = i;
			++copy;
			auto non_const_outer = repr_.erase(i.outer_, i.outer_);
			non_const_outer->second.erase(i.inner_); // Amortised O(1) solution
			return copy;
		}

		auto erase_edge(iterator i, iterator s) noexcept -> iterator {
			while (i != s and i != end()) {
				i = erase_edge(i); // Dist(i, s) * amortised O(1) = O(d) solution
			}
			return i;
		}

		auto clear() noexcept -> void {
			repr_.clear();
			nodes_.clear();
		}

		// Accessors

		[[nodiscard]] auto is_node(N const& value) const noexcept -> bool {
			return nodes_.find(value) != nodes_.end(); // O(log(n)) solution
		}

		[[nodiscard]] auto empty() const noexcept -> bool {
			return nodes_.empty() and repr_.empty();
		}

		[[nodiscard]] auto is_connected(N const& src, N const& dst) const -> bool {
			auto const& src_node = repr_.find(src);
			auto const& dst_node = repr_.find(dst);
			if (src_node != repr_.end() and dst_node != repr_.end()) {
				auto const& edges = src_node->second;
				return edges.find(dst_node->first) != edges.end();
			}
			else {
				throw std::runtime_error("Cannot call gdwg::graph<N, E>::is_connected if src or dst node "
										"don't exist in the graph");
			}
		}

		[[nodiscard]] auto nodes() const -> std::vector<N> {
			auto v = std::vector<N>(nodes_.size());
			std::transform(nodes_.begin(), nodes_.end(), v.begin(), [](auto const& node) {
				return *node;
			}); // O(n) solution
			return v;
		}

		[[nodiscard]] auto weights(N const& src, N const& dst) const -> std::vector<E> {
			// Time complexity
			//        checking nodes exist    - 2 log(n) +
			//        finding first edge      -   log(n) +
			//        subsequent loops        -   e
			//     = O(log(n) + e) solution

			// Check both nodes exist on graph, otherwise throw an exception
			auto const& src_node = repr_.find(src); // O(log(n))
			auto const& dst_node = repr_.find(dst); // O(log(n))
			if (src_node != repr_.end() and dst_node != repr_.end()) {
				auto& edges = src_node->second;
				auto v = std::vector<E>();

				// Finding first edge - O(log(n)) +
				// Subsequent loops   - O(e)
				for (auto it = edges.find(dst_node->first); it != edges.end(); ++it) {
					if (*(it->first) == dst) {
						v.push_back(it->second);
					}
					else {
						// Once end() or the number of stored edges is reached, break loop
						break;
					}
				}
				return v;
			}
			else {
				throw std::runtime_error("Cannot call gdwg::graph<N, E>::weights if src or dst node don't "
										"exist in the graph");
			}
		}

		[[nodiscard]] auto find(N const& src, N const& dst, E const& weight) const -> iterator {
			// Time complexity
			//        find src node   - log(n) +
			//        find exact edge - log(e)
			//    = O(log(n) + log(e)) solution
			auto const& src_node = repr_.find(src); // O(log(n))
			if (src_node != repr_.end()) {
				auto const& edge = src_node->second.find(std::make_pair(dst, weight)); // O(log(e))
				if (edge != src_node->second.end()) {
					return iterator(repr_.end(), src_node, edge);
				}
				else {
					return end();
				}
			}
			else {
				return end();
			}
		}

		[[nodiscard]] auto connections(N const& src) const -> std::vector<N> {
			// Time complexity
			//        find src node        - log(n) +
			//        construct vector     - e +
			//        remove duplicates    - 2e
			//     = O(log(n) + e) solution
			auto const& src_node = repr_.find(src); // O(log(n))
			if (src_node != repr_.end()) {
				// O(e) vector construction
				auto v = std::vector<N>(src_node->second.size());
				std::transform(src_node->second.begin(),
							src_node->second.end(),
							v.begin(),
							[](auto const& set_pair) { return *set_pair.first; });

				// O(2e) remove duplicates
				auto const& last = std::unique(v.begin(), v.end());
				v.erase(last, v.end());

				return v;
			}
			else {
				throw std::runtime_error("Cannot call gdwg::graph<N, E>::connections if src doesn't exist "
										"in the graph");
			}
		}

		// Iterator access

		[[nodiscard]] auto begin() const -> iterator {
			return iterator(repr_.begin(), repr_.end());
		}

		[[nodiscard]] auto end() const -> iterator {
			return iterator(repr_.end(), repr_.end());
		}

		// Comparisons

		[[nodiscard]] auto operator==(graph const& other) const noexcept -> bool {
			// Time complexity
			//        nodes      - n +
			//        edges      - e
			//     = O(n + e) solution
			auto nodes_are_equal =
			std::equal(this->nodes_.begin(),
						this->nodes_.end(),
						other.nodes_.begin(),
						[](auto const& lhs, auto const& rhs) { return *lhs == *rhs; });
			auto edges_are_equal =
			std::equal(this->begin(), this->end(), other.begin(), [](auto const& lhs, auto const& rhs) {
				return lhs.from == rhs.from and lhs.to == rhs.to and lhs.weight == rhs.weight;
			});
			return nodes_are_equal and edges_are_equal;
		}

		// Extractor
		friend auto operator<<(std::ostream& os, graph const& g) -> std::ostream& {
			for (auto const& [from, edges] : g.repr_) {
				os << *from << " (\n";
				for (auto const& [to, weight] : edges) {
					os << "  " << *to << " | " << weight << "\n";
				}
				os << ")\n";
			}
			return os;
		}

		// Iterator
		using iterator = iterator; // custom iterator is defined private

	private:
		// Data Structure and Custom Comparators

		// Allow lexigraphical sorting of pointers based on their underlying values
		struct NodeCompare {
			using is_transparent = void;
			auto operator()(std::shared_ptr<N> const& lhs, std::shared_ptr<N> const& rhs) const -> bool {
				return *lhs < *rhs;
			}
			auto operator()(N const& lhs, std::shared_ptr<N> const& rhs) const -> bool {
				return lhs < *rhs;
			}
			auto operator()(std::shared_ptr<N> const& lhs, N const& rhs) const -> bool {
				return *lhs < rhs;
			}
		};
		struct MapCompare {
			using is_transparent = void;
			auto operator()(N* const& lhs, N* const& rhs) const -> bool {
				return *lhs < *rhs;
			}
			auto operator()(N const& lhs, N* const& rhs) const -> bool {
				return lhs < *rhs;
			}
			auto operator()(N* const& lhs, N const& rhs) const -> bool {
				return *lhs < rhs;
			}
		};
		struct EdgeCompare {
			using is_transparent = void;
			auto operator()(std::pair<N*, E> const& lhs, std::pair<N*, E> const& rhs) const -> bool {
				if (*(lhs.first) == *(rhs.first)) {
					return lhs.second < rhs.second;
				}
				return *(lhs.first) < *(rhs.first);
			}
			auto operator()(std::pair<N, E> const& lhs, std::pair<N*, E> const& rhs) const -> bool {
				if (lhs.first == *(rhs.first)) {
					return lhs.second < rhs.second;
				}
				return lhs.first < *(rhs.first);
			}
			auto operator()(std::pair<N*, E> const& lhs, std::pair<N, E> const& rhs) const -> bool {
				if (*(lhs.first) == rhs.first) {
					return lhs.second < rhs.second;
				}
				return *(lhs.first) < rhs.first;
			}
			auto operator()(N* const& lhs, std::pair<N*, E> const& rhs) const -> bool {
				return *lhs < *(rhs.first);
			}
			auto operator()(std::pair<N*, E> const& lhs, N* const& rhs) const -> bool {
				return *(lhs.first) < *rhs;
			}
		};

		std::set<std::shared_ptr<N>, NodeCompare> nodes_;
		std::map<N*, std::set<std::pair<N*, E>, EdgeCompare>, MapCompare> repr_;

		class iterator {
		public:
			using value_type = graph<N, E>::value_type;
			using reference = value_type;
			using pointer = void;
			using difference_type = std::ptrdiff_t;
			using iterator_category = std::bidirectional_iterator_tag;

			iterator() = default;

			auto operator*() const -> reference {
				return value_type{*outer_->first, *inner_->first, inner_->second};
			}

			auto operator++() -> iterator& {
				++inner_;
				// Goes to next source node if current node has no destination edges
				while (outer_ != end_ and inner_ == outer_->second.end()) {
					++outer_;
					if (outer_ != end_) {
						inner_ = outer_->second.begin();
					}
					else {
						inner_ = inner_it{};
					}
				}
				return *this;
			}

			auto operator++(int) -> iterator {
				auto copy = *this;
				++*this;
				return copy;
			}

			auto operator--() -> iterator& {
				while (outer_ == end_ or inner_ == outer_->second.begin()) {
					--outer_;
					inner_ = outer_->second.end();
				}
				--inner_;
				return *this;
			}

			auto operator--(int) -> iterator {
				auto copy = *this;
				--*this;
				return copy;
			}

			auto operator==(iterator const& other) const -> bool {
				return this->outer_ == other.outer_ and this->inner_ == other.inner_;
			}

		private:
			using outer_it =
			   typename std::map<N*, std::set<std::pair<N*, E>, EdgeCompare>>::const_iterator;
			using inner_it = typename std::set<std::pair<N*, E>, NodeCompare>::const_iterator;

			explicit iterator(outer_it begin, outer_it end)
			: end_{end}
			, outer_{end}
			, inner_{} {
				if (begin != end) {
					while (begin != end) {
						if (not begin->second.empty()) {
							// Finds the first edge to become the begin() iterator
							outer_ = begin;
							inner_ = begin->second.begin();
							break;
						}
						else {
							++begin;
						}
					}
				}
			}

			explicit iterator(outer_it end, outer_it outer, inner_it inner)
			: end_{end}
			, outer_{outer}
			, inner_{inner} {}

			outer_it end_;
			outer_it outer_;
			inner_it inner_;

			friend class graph;
		};
	};

} // namespace gdwg

#endif // GDWG_GRAPH_HPP
