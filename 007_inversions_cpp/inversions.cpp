#include <iostream>
#include <random>
#include <vector>
#include <stack>
#include <array>
#include <chrono>
#include <fstream>

// review of dr. tian's notes. this is from the recursion section

template <typename numeric>
std::vector<numeric> random_std_vector(const size_t elems, const numeric lower, const numeric upper) {
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<numeric> dis(lower, upper);
	
	std::vector<numeric> V(elems);
	for (size_t i = 0; i < elems; i++) {
		V[i] = dis(gen);
	}

	return V;
}

/** Obligatory vector printing function */
template <typename T>
std::vector<T> print_vec(std::vector<T>&& V) {
	typename std::vector<T>::iterator it = V.begin();
	std::cout << "[";
	if (it != V.end()) {	std::cout << (*it);	it++;	}
	for (; it != V.end(); it++) {
		std::cout << ", " << (*it);
	}
	std::cout << "]" << std::endl;
	return V;
}

/** Naive inversion counting implementation. Shortest code though! */
template <typename ordered>
size_t inversions_naive(const std::vector<ordered>& v) {
	const size_t SIZE = v.size();
	size_t inversions = 0;
	for (size_t i = 0; i < SIZE; i++) {
		const ordered& o = v[i];
		for (size_t j = i + 1; j < SIZE; j++) {
			inversions += static_cast<size_t>(o > v[j]);
		}	
	}

	return inversions;
}

/** Helper struct */
template <typename T>
struct sub_vector {
	size_t m_start;
	size_t m_size;

	inline static sub_vector<T> subvec(size_t start, size_t size) {
		return sub_vector {
			start,
			size
		};
	}

	inline T& index(std::vector<T>& parent, const size_t idx) {
		return (parent)[idx + m_start];
	}

	inline const T& index(std::vector<T>& parent, const size_t idx) const {
		return (parent)[idx + m_start];
	}

	inline size_t size() const {
		return m_size;
	}

	inline std::array<sub_vector<T>, 2> halves() const {
		const size_t hs = m_size / 2;
		return {
			sub_vector<T>::subvec(m_start, hs),
			sub_vector<T>::subvec(m_start + hs, m_size - hs)
		};
	}

	inline sub_vector<T> first_half() const {
		return sub_vector<T>::subvec(m_start, m_size / 2);
	}

	inline sub_vector<T> second_half() const {
		const size_t hs = m_size / 2;
		return sub_vector<T>::subvec(m_start + hs, m_size - hs);
	}

	inline bool within(const sub_vector<T>& wrapper) const {
		return this->m_start >= wrapper.m_start && this->m_size <= wrapper.m_size;
	}
};

/** Recursive approach that creates many small vector buffers */
template <typename O>
std::pair<std::vector<O>, size_t> inversions_rec(std::vector<O>&& v) {
	if (v.size() <= 1) { return { v, 0}; }

	const size_t N = v.size();
	const size_t hs = N / 2;

	std::vector<O> h1(v.begin(), v.begin() + hs);
	std::pair<std::vector<O>, size_t> p1 = inversions_rec(std::move(h1));

	std::vector<O> h2(v.begin() + hs, v.end());
	std::pair<std::vector<O>, size_t> p2 = inversions_rec(std::move(h2));

	std::vector<O> ret(N);
	size_t counter = 0;
	size_t i = 0;
	size_t j = 0;

	for (size_t idx = 0; idx < N; idx++) {
		const bool h1_valid = i < p1.first.size();
		const bool h2_valid = j < p2.first.size(); 

		if (h1_valid && h2_valid) {
			const O& p1_val = p1.first[i];
			const O& p2_val = p2.first[j];

			if (p1_val <= p2_val) {
				ret[idx] = p1_val;
				counter += j;
				i += 1;
			} else {
				ret[idx] = p2_val;
				j += 1;
			}

		} else if (h1_valid) {
			ret[idx] = p1.first[i];
			counter += j;
			i += 1;
		} else {
			ret[idx] = p2.first[j];
			j += 1;
		}
	}

	return { std::move(ret), counter + p1.second + p2.second };
}

/** Recursion hub */
template <typename O>
size_t inversions_aight(const std::vector<O>& v) {
	return inversions_rec(std::vector<O>(v)).second;
}

/** O for ordered. I was tired of typing "ordered" everywhere. Uses 1 large buffer instead of creating
 * many small ones. */
template <typename O>
size_t inversions_epic(const std::vector<O>& v) {
	std::vector<O> w(v);

	struct ms_plate {
		sub_vector<O> range;
		bool visited;

		static ms_plate plate(size_t start, size_t size) {
			return ms_plate {
				sub_vector<O>::subvec(start,size),
				false
			};
		}

		static ms_plate from_subvec(sub_vector<O>&& sv) {
			return ms_plate {
				sv,
				false
			};
		}
	};

	std::vector<O> temp_buff(v.size());
	std::stack<ms_plate> subvecs;
	subvecs.push(ms_plate::plate(0,w.size()));
	size_t inversions = 0;

	while (subvecs.size() > 0) {
		ms_plate& p = subvecs.top();

		if (p.range.size() > 1) {
			if (!p.visited) {
				std::array<sub_vector<O>,2> parts = p.range.halves();
				subvecs.push(ms_plate::from_subvec(std::move(parts[0])));
				subvecs.push(ms_plate::from_subvec(std::move(parts[1])));
				p.visited = true;
			} else {
				std::array<sub_vector<O>,2> parts = p.range.halves();
				size_t i = 0;
				size_t j = 0;
				for (size_t idx = 0; idx < p.range.size(); idx++) {
					const bool fh_valid = i < parts[0].size();
					const bool sh_valid = j < parts[1].size();

					if (fh_valid && sh_valid) {
						const O& fh_val = parts[0].index(w,i);
						const O& sh_val = parts[1].index(w,j);

						if (fh_val <= sh_val) {
							temp_buff[idx] = fh_val;
							inversions += j;
							i += 1;
						} else {
							temp_buff[idx] = sh_val;
							j += 1;
						}

					} else if (fh_valid) {
						temp_buff[idx] = parts[0].index(w,i);
						inversions += j;
						i++;
					} else {
						temp_buff[idx] = parts[1].index(w,j);
						j++;
					}
				}

				for (size_t c = 0; c < p.range.size(); c++) {
					p.range.index(w, c) = temp_buff[c];
				}

				subvecs.pop();
			}
		} else {
			subvecs.pop();
		}
	}

	return inversions;
}

template <size_t S>
void to_csv(long long *data, size_t rows, size_t cols, const char* fname, std::array<const char*,S> titles) {
	if (S != rows) {
		std::cout << "array 'titles' size must be equal to `size_t rows` passed in" << std::endl;
		exit(EXIT_FAILURE);
	}

	std::ofstream f(fname);
	
	{
		int local_i = 0;
		for (const char* c : titles) {
			f << c << ((local_i == (rows - 1)) ? "\n" : ",");
			local_i++;
		}

	}

	for (size_t j = 0; j < cols; j++) {
		for (size_t i = 0; i < rows; i++) {
			f << data[i * cols + j] << ((i == (rows - 1)) ? "\n" : ",");
		}
	}
}

int main() {
	using std::chrono::high_resolution_clock;
	using std::chrono::duration_cast;
	using std::chrono::duration;
	using std::chrono::milliseconds;
	
	constexpr size_t intial_size = 11;
	constexpr size_t runs = 1;
	constexpr size_t experiments = 3;
	long long data[experiments][runs];
	
	size_t s = intial_size;
	for (size_t n = 0; n < runs; n++) {
		std::cout << "\n(" << (n+1) << ") INVERSIONS COMPARISON FOR [n = " << s << "] ELEMENTS." << std::endl;
		std::cout << "==============================" << std::endl;
		std::vector<float> v = print_vec(random_std_vector<float>(s, 0.f, 50.f));
		
		{
			const auto t1 = high_resolution_clock::now();
			const size_t inv = inversions_naive(v);
			const auto t2 = high_resolution_clock::now();
	
			const auto ms_int = duration_cast<milliseconds>(t2 - t1);
			std::cout << "\ninversions_naive took " << ms_int.count() << "ms\n";
			std::cout << "# of inversions = " << inv << std::endl;
			data[0][n] = ms_int.count();
		}
		
		{
			const auto t1 = high_resolution_clock::now();
			const size_t inv = inversions_epic(v);
			const auto t2 = high_resolution_clock::now();
	
			const auto ms_int = duration_cast<milliseconds>(t2 - t1);
			std::cout << "\ninversions_epic took " << ms_int.count() << "ms\n";
			std::cout << "# of inversions = " << inv << std::endl;
			data[1][n] = ms_int.count();
		}

		{
			const auto t1 = high_resolution_clock::now();
			const size_t inv = inversions_aight(v);
			const auto t2 = high_resolution_clock::now();
	
			const auto ms_int = duration_cast<milliseconds>(t2 - t1);
			std::cout << "\ninversions_aight took " << ms_int.count() << "ms\n";
			std::cout << "# of inversions = " << inv << std::endl;
			data[2][n] = ms_int.count();
		}

		// {
		// 	const auto t1 = high_resolution_clock::now();
		// 	const size_t inv = inversions_legendary(v);
		// 	const auto t2 = high_resolution_clock::now();
	
		// 	const auto ms_int = duration_cast<milliseconds>(t2 - t1);
		// 	std::cout << "\ninversions_legendary took " << ms_int.count() << "ms\n";
		// 	std::cout << "# of inversions = " << inv << std::endl;
		// 	data[3][n] = ms_int.count();
		// }

		s *= 2;
	}

	std::array<const char*, experiments> cols = {"inversions_naive", "inversions_epic", "inversions_aight"};
	to_csv(&data[0][0], experiments, runs, "./msort_benchmark.csv", cols);
	return EXIT_SUCCESS;	
}
