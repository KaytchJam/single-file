#include <stdio.h>
#include <stdlib.h>

// int test() {
//     FILE* f = fopen("./hello.txt", "r");
//     if (f == NULL) {
//         printf("Error! Failed to open file.");
//         return EXIT_FAILURE;
//     }
    
//     // read file character by character
//     printf("APPROACH 1\n");
//     char c;
//     while ((c = fgetc(f)) != EOF) {
//         putchar(c);
//     }
    
//     fseek(f, 0, SEEK_SET);
    
//     // read file in 4-byte chunks
//     printf("\nAPPROACH 2\n");
//     char buff[4];
//     while (fgets(buff, 4, f) != NULL) {
//         printf("%s\n", buff);
//     }
    
//     fclose(f);
//     return EXIT_SUCCESS;
// }

int main() {
    printf("Reading IHA file\n");
    FILE* ipa_file = fopen("./.iha", "rb");

    if (ipa_file == NULL) {
        printf("Error! Unable to load iha file\n");
        exit(EXIT_FAILURE);
    }

    // get # of elements
    int num_elements = 0;
    fread(&num_elements, sizeof(int), 1, ipa_file);
    printf("There are %d elements in the .iha file.\n", num_elements);
    int exit_loop = 0;

    
    while (1) {
        char user_in = 0;
        printf("Enter 'q' to quit, or 'i' to index into the file: ");
        const int scan_status = scanf("%c", &user_in);
        printf("\n");

        if (scan_status == EOF) {
            printf("Error! Invalid index input.");
            exit(EXIT_FAILURE);
        }

        switch (user_in) {
            case 'i':
                int target_index = 0;
                printf("Enter an index: ");
                const int scan_status = scanf("%d", &target_index);
                printf("\n");

                if (target_index >= num_elements || target_index < 0) {
                    printf("ERROR! Index Out of bounds.");
                    exit(EXIT_FAILURE);
                }

                // target_index plus metadata
                int status = fseek(ipa_file, target_index * sizeof(int) + sizeof(int), SEEK_SET);
                
                int data = -1;
                fread(&data, sizeof(int), 1, ipa_file);
                printf("Target = %d\n", data);              
            break;
            case 'q':
                exit_loop = 1;
            break;
            default:
                break;
        }

        if (exit_loop) {
            break;
        }
    }

    return EXIT_SUCCESS;
}