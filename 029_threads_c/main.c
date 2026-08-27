#include <pthread.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

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

// bool increment_if_less_than(Counter* c, int upper_limit) {
//     bool 
// }

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
    pthread_t* p;
    Counter* c;
    int n;
} ThreadArgs;

/** Adds to the counter until it equals 10 */
void add_till_ten(void* args) {
    ThreadArgs* targs = (ThreadArgs*) args;
    while (get(targs->c) < targs->n) {
        increment(targs->c);
    }
}

typedef struct StringToInt32Result {
    int value;
    bool is_int32;
} StringToInt32Result;

StringToInt32Result string_to_int32(char* number_maybe) {
    StringToInt32Result res;
    res.value = 0;
    res.is_int32 = true;

    if (number_maybe == NULL) {
        res.is_int32 = false;
        return res;
    }

    int64_t value = 0;
    int8_t digit = 0;
    while (*number_maybe != '\0' && digit < 31) {
        const char cur = *number_maybe;
        if ('0' <= cur && cur <= '9') {
            const int int_cur = (int) (cur - '0');
            value = value * 10 + (int64_t) int_cur;
        } else {
            res.is_int32 = false;
            return res;
        }

        digit += 1;
        number_maybe += 1;
    }

    if (digit > 31 || value > (int64_t) INT_MAX) {
        res.is_int32 = false;
        return res;
    }

    res.value = (int32_t) value;
    return res;
}

int main(int argc, char** argv) {
    int N = 100;
    bool arg_set = false;
    for (int i = 0; i < argc; i++) {
        const char* cur = argv[i];
        if (strcmp(cur, "-n") == 0 && i + 1 < argc) {
            StringToInt32Result outcome = string_to_int32(argv[i+1]);
            if (outcome.is_int32) {
                N = outcome.value;
                arg_set = true;
            }
        }
    }

    if (!arg_set) {
        printf("The default count is N = 100. You can set the count 'N' with: ./main -n <count>\n");
    }

    printf("N = %d\n", N);

    // pthread_t p1;
    // pthread_t p2;


    // Counter c = init(0);
    // ThreadArgs args1 = { &p1, &c, N };
    // ThreadArgs args2 = { &p2, &c, N  };

    // pthread_create(&p1, NULL, (void*) add_till_ten, &args1);
    
    // pthread_create(&p2, NULL, (void*) add_till_ten, &args2);

    // pthread_join(p1, NULL);
    // printf("Thread 1 Done.\n");

    // pthread_join(p2, NULL);
    // printf("Thread 2 Done.\n");

    // printf("Final count: %d", c.count);

    return 0;
}