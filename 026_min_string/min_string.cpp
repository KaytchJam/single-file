#include <iostream>

struct Testcase {
    const char* input;
    int answer;
};

int minimum_length(const std::string& str) {
    if (str.size() == 1) {
        return 1;
    }

    constexpr char A = 'A';
    constexpr char B = 'B';
    
    int j = 0; // index in our buffer
    int q = 0;

    // initial check
    if (str[0] == 'A') {
        j += 1;
        q += 1;
    }

    for (int i = 1; i < str.size(); i++) {
        const char prev = str[i - 1];
        const char cur = str[i];

        if (prev == B && cur == A) {
            int l = i - 1;
            int r = i;

            while (l >= 0 && r < str.size() && (str[l] == B && (str[r] == A || str[r] == B))) {
                l -= 1; r += 1;
            }
            
            j += l - q + 1;
            q = r;
            i = r - 1;
        } else if (cur == A && prev == A) {
            j += 1;
            q += 1;
        }
    }

    if ((str.size() - q) % 2 == 0x1) {
        q += 1;
        j += 1;
    }

    return j;
}

int main() {
    Testcase tests[] = {
        {"ABAABAA", 3}, // delete the BA near the beginning and the BA near the end
        {"BA", 0}, // delete the BA
        {"ABB", 1},
        {"BAABB", 1}, // delete the BB at the end, and the BA at the start
        {"ABBAABA", 1}, // delete the inner and outer BA in the middle, than the BA at the end
        {"AAA", 3}, // can't delete 
        {"AABB", 2}, // delete the BB
        {"AABAAA", 4},
        {"AABBB", 3},
        {"BAA", 1}
    };

    const char* pass = "PASS";
    const char* fail = "FAIL";
    bool all_correct = true;

    for (Testcase& t : tests) {
        const int answer = minimum_length(t.input);
        const bool correct = t.answer == answer;

        all_correct = all_correct && correct;
        std::printf("Input: %s, Expected: %d, Actual %d, Status: %s\n", t.input, t.answer, answer, correct ? pass : fail);
    }

    std::printf("\nDid we get all correct? %s!\n", all_correct ? "YES" : "NO");
}