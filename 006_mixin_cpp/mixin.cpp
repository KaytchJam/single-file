#include <iostream>

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
		return (this->rgba >> 24) & 0xFF;
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

	float get_distance() const {
		return this->distance;
	}
}; 

int main() {
	Node<int, Color, Distance> n = Node<int, Color, Distance>(10);
	n.rgba = 0xFF00FFFF;
	n.distance = 100.f;

	std::cout << "The item is: " << n.item << std::endl;
	std::cout << "The RGBA value is: " << n.rgba << std::endl;
	std::cout << "The DISTANCE value is: " << n.get_distance() << std::endl;

	return EXIT_SUCCESS;
}
