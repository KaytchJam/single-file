#include <pthread.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdatomic.h>

typedef struct Counter {
    int count;
    pthread_mutex_t lock;
} Counter;

Counter init(int count) {
    pthread_mutex_t mutex;
    pthread_mutex_init(&mutex, NULL);
    Counter c = { count, mutex };
    return c;
}

bool increment_if_less_than(Counter* c, int upper_limit) {
    bool increment_occurred = false;
    pthread_mutex_lock(&c->lock);
    if (c->count < upper_limit) {
        c->count += 1;
        increment_occurred = true;
    }
    pthread_mutex_unlock(&c->lock);
    return increment_occurred;
}

int increment(Counter* c) {
    pthread_mutex_lock(&c->lock);
    c->count += 1;
    pthread_mutex_unlock(&c->lock);
}

int decrement(Counter* c) {
    pthread_mutex_lock(&c->lock);
    c->count -= 1;
    pthread_mutex_unlock(&c->lock);
}

int get(Counter* c) {
    pthread_mutex_lock(&c->lock);
    const int val = c->count;
    pthread_mutex_unlock(&c->lock);
    return val;
}

int getprint(Counter* c) {
    pthread_mutex_lock(&c->lock);
    printf("Counter value: %d", c->count);
    pthread_mutex_unlock(&c->lock);
}

typedef struct ThreadArgs {
    Counter* c;
    int n;
    int contribution;
} ThreadArgs;

/** Adds to the counter until it equals 10 */
void add_till_n(void* args) {
    ThreadArgs* targs = (ThreadArgs*) args;
    while (increment_if_less_than(targs->c, targs->n)) {
        targs->contribution += 1;
    }
}

/** Return type for stoi. If a failure occurs, `is_int32` will be false. If successful, `is_int32` will be true. */
typedef struct StringToInt32Result {
    int value;
    bool is_int32;
    const char* error;
} StringToInt32Result;

/** My very own stoi. (with no negatives allowed) */
StringToInt32Result string_to_int32(char* number_maybe) {
    StringToInt32Result res;
    res.value = 0;
    res.is_int32 = true;

    if (number_maybe == NULL) {
        res.is_int32 = false;
        res.error = "Input paramter `number_maybe` was NULL.";
        return res;
    }

    int64_t value = 0;
    int8_t digit = 0;
    while (*number_maybe != '\0' && digit < 10) {
        const char cur = *number_maybe;
        if ('0' <= cur && cur <= '9') {
            value = value * 10 + (int64_t) (cur - '0');
        } else {
            res.is_int32 = false;
            res.error = "Input parameter has non-numeric character.";
            return res;
        }

        digit += 1;
        number_maybe += 1;
    }

    if (value > (int64_t) INT32_MAX) {
        res.is_int32 = false;
        res.error = "Input parameter is larger than largest valid 32 bit integer (2147483647).";
        return res;
    }

    res.value = (int32_t) value;
    res.error = "String successful parsed.";
    return res;
}

/** Sums all the contributions from all the threads */
int32_t sum_all_contributions(const ThreadArgs* all_args, const int size) {
    if (all_args == NULL) {
        return -1;
    }

    int32_t total = 0;
    for (int i = 0; i < size; i++) {
        total += all_args[i].contribution;
    }

    return total;
}

int main(int argc, char** argv) {
    // COMMAND LINE ARGUMENT PARSING
    int N = 100; // number to count towards
    int T = 2; // number of threads
    bool arg_set = false;
    for (int i = 0; i < argc; i++) {
        const char* cur = argv[i];
        if (strcmp(cur, "-n") == 0 && i + 1 < argc) {
            const StringToInt32Result outcome = string_to_int32(argv[i+1]);
            if (outcome.is_int32) {
                N = outcome.value;
                arg_set = true;
            } else {
                printf("Error during parse to int32: %s\n", outcome.error);
            }
        } else if (strcmp(cur, "-t") == 0 && i + 1 < argc) {
            const StringToInt32Result outcome = string_to_int32(argv[i+1]);
            if (outcome.is_int32) {
                T = outcome.value;
            }
        }
    }

    if (!arg_set) {
        printf("The default count is N = 100. You can set the count 'N' with: ./main -n <positive 32 byte integer>\n");
    }

    printf("N = %d\n", N);

    pthread_t* threads = (pthread_t*) malloc(sizeof(pthread_t) * T);
    ThreadArgs* all_args = (ThreadArgs*) malloc(sizeof(ThreadArgs) * T);

    if (threads == NULL || all_args == NULL) {
        printf("Failed to allocate the necessary amount of memory...\n");
        return 1;
    }

    Counter c = init(0);
    for (int i = 0; i < T; i++) {
        ThreadArgs* current_args = all_args + i;
        current_args->c = &c;
        current_args->n = N;
        current_args->contribution = 0;
        pthread_create(threads + i, NULL, (void*) add_till_n, current_args);
    }

    for (int i = 0; i < T; i++) {
        pthread_join(threads[i], NULL);
        printf("Thread %d Done. Contributed = %d\n", i + 1, all_args[i].contribution);
    }

    printf("Final count: %d\n", c.count);
    printf("Contribution sum = %d\n", sum_all_contributions(all_args, T));
    free(threads);
    free(all_args);
    return 0;
}