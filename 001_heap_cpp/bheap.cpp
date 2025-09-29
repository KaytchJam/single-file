#include <iostream>
#include <vector>

enum HeapExtrema {
	HE_MAX,
	HE_MIN
};

constexpr HeapExtrema heap_flip(const HeapExtrema he) {
	return HeapExtrema::HE_MAX == he ? HeapExtrema::HE_MIN : HeapExtrema::HE_MAX;
}

// Template Specialization
template <HeapExtrema HE>
bool heap_operator(int a, int b);

// MAX_HEAP Specialization
template <>
bool heap_operator<HeapExtrema::HE_MAX>(int a, int b) {
	return a >= b;
}

// MIN_HEAP Specialization
template <>
bool heap_operator<HeapExtrema::HE_MIN>(int a, int b) {
	return a <= b;
}

// Our HEAP Class/Struct
template <HeapExtrema HE>
struct MHeap {
	std::vector<int> buffer;

	/** Given a vector and an initial node, swap 'keys' such that the heap property is maintained among
	 * the tree starting at `initial node` and all substrees spawning from `initial_node` */
	template <HeapExtrema H>
	static std::vector<int>& heapify(std::vector<int>& data, int initial_node = 0) {
		int honored_one = initial_node;

		// while the stack is nonempty
		while (honored_one != -1) {
			const int cur = honored_one;
			const int left = cur * 2 + 1;
			const int right = cur * 2 + 2;

			int best = cur;
			if (left < data.size() && !heap_operator<H>(data[cur], data[left])) {
				best = heap_operator<H>(data[best], data[left]) ? best : left; 
			}
			
			if (right < data.size() && !heap_operator<H>(data[cur], data[right])) {
				best = heap_operator<H>(data[best], data[right]) ? best : right; 
			}

			honored_one = -1;
			if (best != cur) {
				std::swap(data[cur], data[best]);
				honored_one = best; 
			}
		}

		return data;
	}

	/** Sorts the list in a manner such that the Heap invariant is 
	 * present in the underlying buffer */
	template <HeapExtrema H>
	static std::vector<int>& heap_sort(std::vector<int>& data) {
		const int initial_node = data.size() / 2 - 1;
		for (int i = initial_node; i >= 0; i--) {
			data = MHeap::heapify<H>(data, i);
	       	}
		
		return data;
	}

	MHeap(std::vector<int>&& data) : buffer(data) {
		MHeap::heap_sort<HE>(this->buffer);
	}

	/** Return the heap type of the enum */
	HeapExtrema heap_type() const {
		return HE;
	}

	/** Look at the front element in the queue */
	int peek() const {
		return this->buffer[0];
	}

	/** Exposes the underflying buffer */
	std::vector<int> expose() {
		return this->buffer;
	}

	/** Evaluates to true if the heap is empty. */
	bool empty() const {
		return !(this->buffer.size() > 0);
	}

	/** Extracts the leading value in the queue, and replaces the "root"
	 * of the heap with the next most 'extreme' value */
	int pop_front() {
		int top = this->peek();
		std::swap(this->buffer[0], this->buffer[this->buffer.size()-1]);
		this->buffer.erase(this->buffer.end() - 1);
		this->buffer = MHeap::heapify<HE>(this->buffer, 0);
		return top;
	}
};

/* Does what it says. Prints da vector */
template <typename T>
std::vector<T> print_vector(std::vector<T>&& v) {
	std::cout << "[ ";
	for (const T& item : v) {
		std::cout << item << " ";	
	}
	std::cout << "]" << std::endl;

	return v;
}

std::vector<int> enumerate_vec(
		const int start,
	       	const int stop,
	       	const bool inclusive = false
) {
	if (stop < start) {
		std::cout << "stop must be greater than or equal to stop" << std::endl;
		abort();
	}

	const int size = stop - start + static_cast<int>(inclusive);
	std::vector<int> buff;
	buff.reserve(size);

	for (int i = 0; i < size; i++) {
		buff.push_back(i + start);
	}

	return buff;
};

int main() {
	std::cout << "PRIOR: ";
	std::vector<int> f = print_vector(enumerate_vec(10, 30));
	constexpr HeapExtrema EXTREMA = HeapExtrema::HE_MAX;
	MHeap<EXTREMA> my_heap(std::move(f));

	std::cout << "POST: ";
	f = print_vector(my_heap.expose());

	while (!my_heap.empty()) {
		int top = my_heap.pop_front();
		std::cout << top << std::endl;
	}

	return EXIT_SUCCESS;
}
