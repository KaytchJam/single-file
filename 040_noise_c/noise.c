#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

/** General type for interpolating functions */
typedef float (*Interpolator)(float a, float b, float t);

/** Linear interpolation. */
float lerp(float a, float b, float t) {
    return (1.f - t) * a + t * b;
}

/** help */
float smoothstep_helper(float t) {
    return 3.f * (t*t) - 2.f * (t*t*t);
}

/** Cubic hermit interpolation. Assumes `0 <= t <= 1`. */
float smoothstep(float a, float b, float t) {
    return (1.f - smoothstep_helper(t)) * a + smoothstep_helper(t) * b;
}

/** Simple type storing a pointer to a float buffer and the size of said buffer. */
typedef struct NoiseBuffer {
    float* data;
    size_t N;
} NoiseBuffer;

/** Creates a new `NoiseBuffer` struct with `NoiseBuffer::data` pointing to `float* data`, and
 * with a size of `size_t n`. */
NoiseBuffer noise_buffer_new(float* data, size_t n) {
    NoiseBuffer buffer;
    buffer.data = data;
    buffer.N = n;
    return buffer;
}

/** Creates a new `NoiseBuffer` and allocates n floats of memory for it. If the n floats of 
 * memory cannot be allocated, a `NoiseBuffer` with `data == NULL` and `n == 0` is returned. */
NoiseBuffer noise_buffer_alloc(size_t n) {
    NoiseBuffer buffer = noise_buffer_new(NULL, 0);
    if (n > SIZE_MAX) {
        return buffer;
    }

    buffer.data = (float*) calloc(n, sizeof(float));
    if (buffer.data == NULL) {
        return buffer;
    }

    buffer.N = n;
    return buffer;
}

/** Frees the data pointed to by the noise buffer. If the free was unsuccessful,
 * e.g. `NoiseBuffer::data == NULL`, then `false` is returned. Additionally, the input
 * `NoiseBuffer buffer` will have `buffer->data` set to `NULL` and `buffer->N` set to 0. */
bool noise_buffer_free(NoiseBuffer* buffer) {
    bool successful_free = false;
    if (buffer != NULL && buffer->data != NULL) {
        free(buffer->data);
        successful_free = true;
    }

    buffer->data = NULL;
    buffer->N = 0;
    return successful_free;
}

/** Comma-seperated print of the noise buffer */
void noise_buffer_print(NoiseBuffer buffer) {
    if (buffer.data == NULL) {
        return;
    }

    printf("[");
    if (buffer.N > 0) {
        printf("%f", buffer.data[0]);
    }

    for (size_t i = 1; i < buffer.N; i += 1) {
        printf(",%f", buffer.data[i]);
    }

    printf("]");
}

/** Returns a random number between 0-1 */
float unit_rand() {
    return (float) rand() / (float) RAND_MAX;
}

/** What could this possibly do? */
size_t max(const size_t a, size_t b) {
    return a > b ? a : b;
}

/** Interpolates a range within the NoiseBuffer. */
void interpolate_and_add_range(NoiseBuffer nb, const size_t start, const size_t end, const float y1, const float y2, const Interpolator interpolate) {
    const float x1 = (float) start;
    const float x2 = (float) end;
    const float dx = end - start;

    for (size_t x = start; x < end && x < nb.N; x += 1) {
        const float t = ((float) x - x1) / dx;
        nb.data[x] += interpolate(y1, y2, t);
    }
}

/** Generate `num_values` values of "smooth noise". */
NoiseBuffer generate_noise_1d(size_t num_values, size_t octaves, float amplitude, float shift, size_t initial_period, Interpolator interpolate) {
    NoiseBuffer nb = noise_buffer_alloc(num_values);
    if (nb.data == NULL) {
        printf("Was unable to allocate enough memory to use as a buffer for the noise values.\nExiting.");
        exit(EXIT_FAILURE);
    }

    // Successively add octaves of frequencies to our NoiseBuffer
    size_t period = initial_period;
    for (size_t o = 0; o < octaves; o++) {
        float prev = amplitude * (unit_rand() + shift);

        size_t i = period;
        while (i < num_values) {
            const float cur = amplitude * (unit_rand() + shift);
            interpolate_and_add_range(nb, i - period, i, prev, cur, interpolate);

            prev = cur;
            i += period;
        }

        // edge-case to fill in remainder
        const float cur = amplitude * (unit_rand() + shift);
        interpolate_and_add_range(nb, i - period, i, prev, cur, interpolate);

        period = max(period / 2, 1);
        amplitude /= 2.f;
    }
    
    return nb;
}

/** Attempts to read input argument `i` and parse it into an `int32_t`. If a failure occurs, we instead
 * return `otherwise`. */
int32_t read_and_set_int(const int argc, char* argv[], const int i, const int32_t otherwise) {
    if (i >= argc) {
        return otherwise;
    }
    
    char* input = argv[i];
    char* endptr;
    const int32_t value = (int32_t) strtol(input, &endptr, 10);
    
    if (endptr == input) {
        return otherwise;
    }

    return value;
}

/** Attempts to read input argument `i` and parse it into a `float`. If a failure occurs, we instead
 * return `otherwise`. */
float read_and_set_float(const int argc, char* argv[], const int i, const float otherwise) {
    if (i >= argc) {
        return otherwise;
    }
    
    char* input = argv[i];
    char* endptr;
    const float value = (float) strtof(input, &endptr);
    
    if (endptr == input) {
        return otherwise;
    }

    return value;
}

/** Attempts to read input argument `i` and parse it into a `char*`. If a failure occurs, we instead
 * return `otherwise`. */
const char* read_and_set_str(const int argc, char* argv[], const int i, const char* otherwise) {
    if (i >= argc) {
        return otherwise;
    }
    
    char* input = argv[i];
    return input;
}

/** Clamps input `str` to a string compatible with `str_to_interpolator`, being
 * "lerp", and "smoothstep". Defaults to "lerp" if not compatible. */
const char* constrain_interpolator_str(const char* str) {
    if (strcmp(str, "lerp") == 0) {
        return "lerp";
    } else if (strcmp(str, "smoothstep") == 0) {
        return "smoothstep";
    }

    return "lerp";
}

/** Maps an string to an interpolating function. */
Interpolator str_to_interpolator(const char* str) {
    if (strcmp(str, "lerp") == 0) {
        return lerp;
    } else if (strcmp(str, "smoothstep") == 0) {
        return smoothstep;
    }

    // default
    return lerp;
}

int main(int argc, char* argv[]) {
    int32_t seed = time(0);  // -s <int>: random seed to use when generating random values
    size_t r = 1;  // -r <int>: increases the resolution of the signal/noise by scaling both `n` and `p`, if you modify the resolution be sure to apply time compression if graphing the list
    size_t n = 50;  // -n <int>: initial number of records we have in our buffer
    size_t o = 4;   // -o <int>: octaves of diminishing frequencies to apply when generating our signal
    size_t p = 3;   // -p <int>: the period of our fundamental frequency
    float a = 5;    // -a <float>: amplitude of our random values that are initially between 0 and 1
    float t = -0.5; // -t <float>: vertical translation of initial random values between 0 and 1
    const char* i = "lerp"; // -i <"lerp" | "smoothstep">: the interpolating function apply. supports "lerp" and "smoothstep" as arguments

    for (int32_t argi = 1; argi < argc; argi++) {
        char* cur = argv[argi];
        if (strcmp(cur, "-s") == 0) {
            seed = read_and_set_int(argc, argv, argi + 1, seed);
        } else if (strcmp(cur, "-r") == 0) {
            r = max((size_t) read_and_set_int(argc, argv, argi + 1, r), 1);
        } else if (strcmp(cur, "-n") == 0) {
            n = (size_t) read_and_set_int(argc, argv, argi + 1, n);
        } else if (strcmp(cur, "-o") == 0) {
            o = read_and_set_int(argc, argv, argi + 1, o);
        } else if (strcmp(cur, "-p") == 0) {
            p = max(1, read_and_set_int(argc, argv, argi + 1, p));
        } else if (strcmp(cur, "-a") == 0) {
            a = read_and_set_float(argc, argv, argi + 1, a);
        } else if (strcmp(cur, "-t") == 0) {
            t = read_and_set_float(argc, argv, argi + 1, t);
        } else if (strcmp(cur, "-i") == 0) {
            i = constrain_interpolator_str(read_and_set_str(argc, argv, argi + 1, i));
        }
    }

    Interpolator fi = str_to_interpolator(i);
    n *= r;
    p *= r;

    printf("seed = %d\nresolution = %d\nnum_values = %d\noctaves = %d\nperiod = %d\namplitude = %f\nvertical shift = %f\ninterpolator = %s\n", seed, r, n, o, p, a, t, i);
    srand(seed);

    NoiseBuffer buffer = generate_noise_1d(n, o, a, t, p, fi);
    noise_buffer_print(buffer);
    noise_buffer_free(&buffer);

    return EXIT_SUCCESS;
}