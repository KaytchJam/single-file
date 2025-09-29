
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <string>
#include <limits>

/** Igneous rock Enum */
enum IGNEOUS_ROCK {
    FELSIC = 0,
    INTERMEDIATE = 1,
    MAFIC = 2,
    ULTRAMAFIC = 3
};

/** Pass in an Igneous Rock enum and return its string representation */
const char* ig2str(IGNEOUS_ROCK rock) {
    switch (rock) {
        case FELSIC: return "felsic";
        case INTERMEDIATE: return "intermediate";
        case MAFIC: return "mafic";
        case ULTRAMAFIC: return "ultramafic";
        default: return "";
    }
}

/** Simple interval representation. */
struct interval {
  double low;
  double high;
  
  double mean() const {
      return (this->high + this->low) / 2.0;
  }
};

/** Pass in an igneous rock and return the silica content range of the rock 
 * as an interval. */
interval ig2interval(IGNEOUS_ROCK rock) {
    switch (rock) {
        case FELSIC: return {66.0, 76.0};
        case INTERMEDIATE: return {52.0, 66.0};
        case MAFIC: return {45.0, 52.0};
        case ULTRAMAFIC: return {0.0, 45.0};
        default: return {0.0, 0.0};
    }
}

/** Pass in a silica content percentage and return the igenous rock that said
 * percentage is closest to. */
const char* silica_rock_thresh(float silica_percent_content) {
    const IGNEOUS_ROCK types[4] = {FELSIC, INTERMEDIATE, MAFIC, ULTRAMAFIC };
    double means[4] = {};
    
    // get means of all intervals
    for (int i = 0; i < 4; i++) {
        means[i] = ig2interval(types[i]).mean();
    }
    
    double dist = std::numeric_limits<double>::max();
    int closest = -1;
    
    // which mean is our percentage "closest" to
    for (int i = 0; i < 4; i++) {
        double dist_local = abs(means[i] - silica_percent_content);
        if (dist_local < dist) {
            dist = dist_local;
            closest = i;
        }
    }
    
    return ig2str(types[closest]);
}

int main() {
    srand(time(0)); // Seed the random number generator
    double random_thresh = (double) (rand() % 101);
    std::cout << "SILICA PERCENT CONTENT = " << random_thresh << "%" << std::endl;
    std::cout << "THIS ROCK IS: " << silica_rock_thresh(random_thresh) << std::endl;
    return 0;
}