#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <array>
#include <optional>
#include <cstdint>

/** DUMB Branchless programming shit. Just use a ternary gang */
template<typename T>
T& branchless_select(const bool b, T& true_item, T& false_item) noexcept {
    return *reinterpret_cast<T*>(
        (b * reinterpret_cast<std::uintptr_t>(&true_item)) 
        + (!b * reinterpret_cast<std::uintptr_t>(&false_item)));
}

/** Make a random std::vector of size `size`. Random values are in the range [0,ub) */
std::vector<int> random_data(size_t size, int ub = 100) {
    std::vector<int> data(size);
    srand((unsigned int) time(0));
    for (int i = 0; i < size; i++) {
        int num = rand() % ub;
        data[i] = num;
    }
    
    return data;
}

/** Prints da vector */
void print_vector(const std::vector<int>& v) {
    std::cout << "[ ";
    for (int i : v) {
        std::cout << i << " ";
    }
    std::cout << "]" << std::endl;
}

/** Stores an index along with some element `elem`. The index only truly has meaning when
 * associated with some range/container. */ 
template <typename T>
struct IndexWrapper {
    size_t index;
    T elem;

    static IndexWrapper<T> wrap(size_t index, T&& item) {
        return IndexWrapper {
            index,
            item
        };
    }
    
    T& operator*() {
        return this->elem;
    }
    
    T* operator->() {
        return &this->elem;
    }
};

/** An intrusive Linked List node. Only truly has meaning when associated with some container/range. */
template <typename T>
struct IllNode {
    T item;
    std::array<std::optional<size_t>, 2> links;
    
    static IllNode<T> wrap(T&& item, size_t prev, size_t next) {
        return IllNode {
            item,
            { prev, next }
        };
    }
    
    static IllNode<T> linkless(T&& item) {
        return IllNode {
            item,
            { std::nullopt, std::nullopt }
        };
    }
};

/** An IndexPolicy struct that returns 0 on map */
struct DefaultIP {
    static size_t map(size_t idx) {
        return 0;
    }
};

/** An IndexPolicy struct that returns the passed in value on map */
struct IdentityIP {
    static size_t map(size_t idx) {
        return idx;
    }
};

/** Takes in an std::vector<T> r_value and converts it to std::vector<IndexWrapper<T>> */
template <typename T, typename IndexPolicy = IdentityIP>
std::vector<IndexWrapper<T>> iwrapper_wrap_vec(std::vector<T>&& v) {
    std::vector<IndexWrapper<T>> w;
    size_t i = 0;
    for (T& elem : v) {
        w.push_back(
            IndexWrapper<T>::wrap(IndexPolicy::map(i), std::move(elem))
        );
        i += 1;
    }
    
    return w;
}

/** Takes in an std::vector<T> r_value and converts it to std::vector<IllNode<T>> */
template <typename T>
std::vector<IllNode<T>> illnode_wrap_vec(std::vector<T>&& v) {
    std::vector<IllNode<T>> w;
    size_t i = 0;
    for (T& elem : v) {
        w.push_back(
            IllNode<T>::linkless(std::move(elem))
        );
    }
    
    return w;
}

// Most of the shit above did not matter LMAOOOOOOOO
// Main idea here? Linked List based centroid lists
// `data` holds the I(ntrusive)l(inked)l(ist)Nodes, which are chained together

/** Actual KMeansClustering but just for 1d data. */
template <size_t clusters>
class KMeansClustering {
private:
    std::array<int, clusters> centroids;
    std::array<std::optional<size_t>, clusters> heads;
    std::vector<IllNode<int>> data;

    // need some initial centroid selection policy. For now I'll choose the first "cluster" items in "data"
    void init_centroids() {
        const size_t data_size = this->data.size();
        size_t data_index = 0;

        while (data_index < clusters && data_index < data_size) {
            //this->heads[data_index] = data_index;
            this->centroids[data_index] = this->data[data_index].item;
            data_index += 1;
        }
    }

    /** Given some int `item` returns the nearest cluster to said item */
    size_t nearest_centroid(const int item) const {
        IndexWrapper<int> min_distance = IndexWrapper<int>::wrap(0, std::numeric_limits<int>::max());

        for (size_t i = 0; i < clusters; i++) {
            const int centroid_value = centroids[i];
            int distance = std::abs(centroid_value - item);
            min_distance = branchless_select(distance <= min_distance.elem, IndexWrapper<int>::wrap(i, std::move(distance)), min_distance);
        }

        return min_distance.index;
    }

    /** Adds a node to a cluster */
    void add_node_to_cluster(IllNode<int>& n, const size_t node_index, const size_t cluster_index) {
        std::optional<size_t> prev_head = heads[cluster_index];
        if (prev_head) {
            const size_t prev_head_node_idx = *prev_head;
            IllNode<int>& prev_head_node = this->data[prev_head_node_idx];
            prev_head_node.links[0] = node_index;
            n.links[1] = prev_head_node_idx;
        } else {
            n.links[1] = std::nullopt;
        }

        heads[cluster_index] = node_index;
        n.links[0] = std::nullopt;
    }

    /** Should really be called "assign clusters" but this more or less matches each item to one of
     * the current centroids. */
    void assign_centroids() {
        const size_t data_size = this->data.size();
        size_t data_index = 0;

        while (data_index < data_size) {
            IllNode<int>& item_at = this->data[data_index];
            const size_t cluster_idx = this->nearest_centroid(item_at.item);
            this->add_node_to_cluster(item_at, data_index, cluster_idx);
            data_index += 1;
        }
    }

    /** Averages all values in a given cluster. */
    int average_cluster(IllNode<int>& cluster_head) {
        IllNode<int>* cur_node = &cluster_head;
        size_t cluster_length = 1;
        int sum = cur_node->item;
        while (cur_node->links[1]) {
            cur_node = &this->data[*cur_node->links[1]];
            sum += cur_node->item;
            cluster_length += 1;
        }

        return sum / (int) cluster_length;
    }

    /** Updates the centroids */
    std::array<int, clusters> update_centroids() {
        std::array<int, clusters> new_centroids = {};
        size_t cluster_idx = 0;

        for (size_t i = 0; i < clusters; i++) {
            if (this->heads[i]) {
                new_centroids[i] = average_cluster(this->data[*this->heads[i]]);
            } else {
                new_centroids[i] = this->centroids[i];
            }
        }

        return new_centroids;
    }

    /** Checks if the previous centroids are equal to the current centroids */
    bool centroids_the_same(const std::array<int, clusters>& new_centroids) {
        for (int c = 0; c < clusters; c++) {
            if (new_centroids[c] != this->centroids[c]) {
                return false;
            }
        }

        return true;
    }

    /** Updates the clusters */
    void update_clusters() {
        for (size_t cluster = 0; cluster < clusters; cluster++) {

            if (this->heads[cluster]) {
                std::optional<size_t> next_node = this->heads[cluster];

                while (next_node) {
                    const size_t cur_node_idx = *next_node;
                    IllNode<int>& cur_node = this->data[cur_node_idx];
                    next_node = cur_node.links[1];

                    size_t cluster_idx = nearest_centroid(cur_node.item);

                    // node removal (from cluster) process
                    if (cluster_idx != cluster) {
                        std::optional<size_t>* prev_target = nullptr;
                        
                        // previous is header
                        if ((*this->heads[cluster]) == cur_node_idx) {
                            prev_target = &this->heads[cluster];
                            if (next_node) {
                                this->data[*next_node].links[0] = std::nullopt;
                            }
                        } else {
                            prev_target = &this->data[*cur_node.links[0]].links[1];
                            if (next_node) {
                                this->data[*next_node].links[0] = cur_node.links[0];
                            }
                        }
                        
                        *prev_target = next_node;
                        add_node_to_cluster(cur_node, cur_node_idx, cluster_idx);
                    }
                }
            }
        }
    }

public:
    KMeansClustering(std::vector<int>&& data): centroids({}), heads({}) {
        this->data = illnode_wrap_vec(std::move(data));
        this->init_centroids();
        this->assign_centroids();

        
        std::array<int, clusters> new_centroids = this->update_centroids();
        while (!this->centroids_the_same(new_centroids)) {
            this->centroids = new_centroids;
            //print_centroids();
            this->update_clusters();
            new_centroids = this->update_centroids();
        }
    }

    /** Collects the clusters as an array of std::vectors */
    std::array<std::vector<int>, clusters> collect_clusters() {
        std::array<std::vector<int>, clusters> out = {}; 
        size_t cluster_idx = 0;

        for (const std::optional<size_t>& head_link : this->heads) {
            std::optional<size_t> next_node = head_link;
            while (next_node) {
                IllNode<int>& cur_node = this->data[*next_node];
                out[cluster_idx].push_back(cur_node.item);
                next_node = cur_node.links[1];
            }
            cluster_idx += 1;
        }

        return out;
    }

    /** Get the centroid of cluster index i */
    int get_centroid(size_t i) {
        return this->centroids[i];
    }
};

int main() {
    std::vector<int> data = random_data(30);

    std::cout << "INITIAL DATA: ";
    print_vector(data);
    std::cout << std::endl;

    constexpr size_t CLUSTERS = 3;
    KMeansClustering<CLUSTERS> cluster_model(std::move(data));
    std::array<std::vector<int>, CLUSTERS> clusters = cluster_model.collect_clusters();

    size_t i = 1;
    for (const std::vector<int>& cluster : clusters) {
        std::cout << "CLUSTER " << i << ": CENTROID = " << cluster_model.get_centroid(i - 1) << ", MEMBERS = ";
        print_vector(cluster);
        i += 1;
    }
    return 0;
}
