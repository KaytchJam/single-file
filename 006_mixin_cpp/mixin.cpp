#include <iostream>
#include <vector>
#include <cstdlib>

template<typename T, typename ...Mixins>
struct Node : public Mixins... {
	T item;
	Node() : Mixins()..., item() {}
	Node(T content) : Mixins()..., item(content) {}
};

struct Color {
	uint32_t rgba;
	Color() : rgba(0) {}

	uint8_t get_red() const {
		return (this->rgba >> 24);
	}

	uint8_t get_green() const {
		return (this->rgba >> 16) & 0xFF;
	}
	
	uint8_t get_blue() const {
		return (this->rgba >> 8) & 0xFF;
	}

	uint8_t get_alpha() const {
		return (this->rgba & 0xFF);
	}

};

struct Distance {
	float distance;
	Distance() : distance(0.f) {}
	Distance(float f) : distance(f) {}

	float get_distance() const {
		return this->distance;
	}

	void set_distance(float f) {
		this->distance = f;
	}
}; 

struct IndexLinks {
	int prev;
	int next;

	IndexLinks() : prev(0), next(0) {}
	IndexLinks(int p, int n) : prev(p), next(n) {}
};

int main() {
	typedef Node<char,Distance,Color,IndexLinks> NodeA;
	std::vector<NodeA> V;

	// constants
	uint32_t colors[3] = {0xFF8A2BFF, 0x73CAFFFF, 0x888888FF};
	IndexLinks links[3] = {{-1, 2}, {2, -1}, {0, 1}};
	Distance d[3] = {2.5f, 3.7f, 11.2f};

	// populate the vector
	for (int i = 0; i < 3; i++) {
		V.push_back(NodeA());
		V[i].item = (char) (i + 0x41);
		V[i].distance = d[i].distance;
		V[i].rgba = colors[i];
		V[i].prev = links[i].prev;
		V[i].next = links[i].next;
	}

	// lambda for printing
	auto print_node_a = [](const NodeA& n) -> const NodeA& {
		std::cout << "ITEM: " << n.item << std::endl;
		std::cout << "DISTANCE (from what?): " << n.get_distance() << " units" << std::endl;
		std::cout << "RED INTENSITY: " << (int) n.get_red() << std::endl;
		std::cout << "GREEN INTESNITY: " << (int) n.get_green() << std::endl;
		std::cout << "BLUE INTENSITY: " << (int) n.get_blue() << std::endl;
		return n;
	};

	// Use the IndexLinks mixin to traverse the vector like a linked list
	int ptr = 0;
	while (ptr != -1) {
		const NodeA& n = print_node_a(V[ptr]);
		std::cout << std::endl;
		ptr = n.next;
	}
	return EXIT_SUCCESS;
}
