#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) < (b) ? (a) : (b))

int bin_pow(int v, int p) {
    int out = 1;
    while (p > 0) {
        if (p & 0x1) {
            out *= v;
        }
        v *= v;
        p >>= 1;
    }
    
    return out;
}

typedef struct MSSResult {
    bool ok;
    int maximum_sum;
} MSSResult;

MSSResult maximum_subarray_sum(const int* arr, int n) {
    MSSResult res;
    if (arr == NULL || n <= 0) {
        res.ok = false;
        return res;
    }
    
    int prev = arr[0];
    res.ok = true;
    res.maximum_sum = prev;
    
    for (int i = 1; i < n; i++) {
        const int cur = arr[i];
        prev = max(prev + cur, cur);
        res.maximum_sum = max(res.maximum_sum, prev);
    }
    
    return res;
}

// [1, 2, 3, 4, 5] -> []
void rotate(int* arr, int n, int k) {
    int* rotated = (int*) malloc(n);
    k = k % n;
    int idx = n - k;
    for (int i = 0; i < k; i++) {
        rotated[idx] = arr[i];
        idx = (idx + 1) % n;
    }
    
    memcpy(rotated, arr, n);
    free(rotated);
}

// add u to v, k times
int add_k(int v, int u, int k) {
    const int og_k = k;
    int i = 0;
    while (k > 0) {
        if (k & 0x1) {
            v += u;
        }
        u += u;
        k >>= 1;
        i += 1;
    }
    
    printf("Expected Iterations: %d, Actual Iterations: %d\n", og_k, i);
    
    return v;
}

int gcd_naive(int a, int b) {
    if (a == 0) {
        return b;
    } else if (b == 0) {
        return a;
    }
    
    int smaller = min(a,b);
    int k = 1;
    int candidate = 0;
    while (k <= smaller) {
        if (a % k == 0 && b % k == 0) {
            candidate = k;
        }
        
        k += 1;
    }
    
    return candidate;
}

int gcd(int a, int b) {
    while (b) {
        a = a % b;
        const int temp = b;
        b = a;
        a = temp;
    }
    
    return a;
}

int lcm(int a, int b) {
    return a / gcd(a,b) * b;
}

int main() {
    // Write C code here
    int res = bin_pow(3, 3);
    printf("Binary Pow Result: %d\n", res);
    
    int arr[] = {10, 15, -25, 2, -5, -8, 4, 6, 10, -11};
    int n = sizeof(arr) / sizeof(int);
    MSSResult res2 = maximum_subarray_sum(arr, n);
    
    if (res2.ok) {
        printf("Maximum Subarray Sum Result: %d\n", res2.maximum_sum);
    }
    
    printf("Add k: %d\n", add_k(20, 3, 100));
    printf("GCD Naive: %d\n", gcd_naive(44, 20));
    printf("GCD: %d\n", gcd(44, 20));
    printf("LCM: %d\n", lcm(36, 48));
    
    return 0;
}