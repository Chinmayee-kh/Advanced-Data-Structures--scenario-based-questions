#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#define SEQUENCE_LENGTH 100000
#define ALPHABET_SIZE 4
#define MAX_CODE_LENGTH 10
const char BASES[ALPHABET_SIZE] = {'A', 'T', 'C', 'G'};
typedef struct HuffmanNode {
    char symbol;
    int frequency;
    struct HuffmanNode *left;
    struct HuffmanNode *right;
} HuffmanNode;
HuffmanNode* create_node(char symbol, int frequency) {
    HuffmanNode *node = malloc(sizeof(HuffmanNode));
    if (node == NULL) {
        printf("Memory allocation failed.\n");
        exit(1);
    }
    node->symbol = symbol;
    node->frequency = frequency;
    node->left = NULL;
    node->right = NULL;
    return node;
}
void find_two_smallest(
    HuffmanNode *nodes[],
    int count,
    int *first,
    int *second
) {
    *first = -1;
    *second = -1;
    for (int i = 0; i < count; i++) {
        if (nodes[i] == NULL) {
            continue;
        }
        if (*first == -1 ||
            nodes[i]->frequency < nodes[*first]->frequency) {

            *second = *first;
            *first = i;
        }
        else if (*second == -1 ||
                 nodes[i]->frequency < nodes[*second]->frequency) {

            *second = i;
        }
    }
}
HuffmanNode* build_huffman_tree(int frequencies[]) {
    HuffmanNode *nodes[ALPHABET_SIZE * 2];
    int count = 0;
    for (int i = 0; i < ALPHABET_SIZE; i++) {
        nodes[count] = create_node(BASES[i], frequencies[i]);
        count++;
    }
    while (count > 1) {
        int first;
        int second;
        find_two_smallest(nodes, count, &first, &second);
        HuffmanNode *left = nodes[first];
        HuffmanNode *right = nodes[second];
        HuffmanNode *parent =
            create_node('\0',
                        left->frequency + right->frequency);
        parent->left = left;
        parent->right = right;
        nodes[first] = parent;
        if (second < count - 1) {
            nodes[second] = nodes[count - 1];
        }
        count--;
    }
    return nodes[0];
}
void generate_codes(
    HuffmanNode *root,
    char current_code[],
    int depth,
    char codes[][MAX_CODE_LENGTH]
) {
    if (root == NULL) {
        return;
    }
    if (root->left == NULL && root->right == NULL) {
        current_code[depth] = '\0';
        for (int i = 0; i < ALPHABET_SIZE; i++) {
            if (BASES[i] == root->symbol) {
                strcpy(codes[i], current_code);
                break;
            }
        }
        return;
    }
    current_code[depth] = '0';
    generate_codes(
        root->left,
        current_code,
        depth + 1,
        codes
    );
    current_code[depth] = '1';
    generate_codes(
        root->right,
        current_code,
        depth + 1,
        codes
    );
}
int base_index(char base) {
    for (int i = 0; i < ALPHABET_SIZE; i++) {
        if (BASES[i] == base) {
            return i;
        }
    }
    return -1;
}
char* encode_sequence(
    const char *sequence,
    char codes[][MAX_CODE_LENGTH],
    long *encoded_length
) {
    long capacity =
        (long)SEQUENCE_LENGTH * MAX_CODE_LENGTH + 1;
    char *encoded = malloc(capacity);
    if (encoded == NULL) {
        printf("Memory allocation failed.\n");
        exit(1);
    }
    long position = 0;
    for (int i = 0; i < SEQUENCE_LENGTH; i++) {
        int index = base_index(sequence[i]);
        if (index == -1) {
            free(encoded);
            return NULL;
        }
        int code_length = strlen(codes[index]);
        for (int j = 0; j < code_length; j++) {
            encoded[position++] = codes[index][j];
        }
    }
    encoded[position] = '\0';
    *encoded_length = position;
    return encoded;
}
char* decode_sequence(
    const char *encoded,
    long encoded_length,
    HuffmanNode *root
) {
    char *decoded =
        malloc((SEQUENCE_LENGTH + 1) * sizeof(char));

    if (decoded == NULL) {
        printf("Memory allocation failed.\n");
        exit(1);
    }
    long decoded_position = 0;
    HuffmanNode *current = root;
    for (long i = 0; i < encoded_length; i++) {
        if (encoded[i] == '0') {
            current = current->left;
        }
        else if (encoded[i] == '1') {
            current = current->right;
        }
        else {
            free(decoded);
            return NULL;
        }
        if (current->left == NULL &&
            current->right == NULL) {
            decoded[decoded_position++] = current->symbol;
            current = root;
        }
    }
    decoded[decoded_position] = '\0';
    return decoded;
}

void free_tree(HuffmanNode *root) {

    if (root == NULL) {
        return;
    }

    free_tree(root->left);
    free_tree(root->right);

    free(root);
}

void generate_sequence(
    char *sequence,
    double probabilities[]
) {
    for (int i = 0; i < SEQUENCE_LENGTH; i++) {

        double r =
            (double)rand() / (double)RAND_MAX;

        if (r < probabilities[0]) {
            sequence[i] = 'A';
        }
        else if (r < probabilities[0] + probabilities[1]) {
            sequence[i] = 'T';
        }
        else if (r < probabilities[0] +
                         probabilities[1] +
                         probabilities[2]) {
            sequence[i] = 'C';
        }
        else {
            sequence[i] = 'G';
        }
    }

    sequence[SEQUENCE_LENGTH] = '\0';
}

void count_frequencies(
    const char *sequence,
    int frequencies[]
) {
    for (int i = 0; i < ALPHABET_SIZE; i++) {
        frequencies[i] = 0;
    }

    for (int i = 0; i < SEQUENCE_LENGTH; i++) {

        int index = base_index(sequence[i]);

        if (index != -1) {
            frequencies[index]++;
        }
    }
}

void run_experiment(
    const char *experiment_name,
    double probabilities[]
) {
    printf("\n");
    printf("%s\n", experiment_name);


    char *sequence =
        malloc((SEQUENCE_LENGTH + 1) * sizeof(char));

    if (sequence == NULL) {
        printf("Memory allocation failed.\n");
        exit(1);
    }

    generate_sequence(sequence, probabilities);

    int frequencies[ALPHABET_SIZE];

    count_frequencies(sequence, frequencies);

    printf("\nMeasured frequencies:\n");

    for (int i = 0; i < ALPHABET_SIZE; i++) {

        double percentage =
            (100.0 * frequencies[i]) / SEQUENCE_LENGTH;

        printf(
            "%c: %d (%.2f%%)\n",
            BASES[i],
            frequencies[i],
            percentage
        );
    }

    HuffmanNode *root =
        build_huffman_tree(frequencies);

    char codes[ALPHABET_SIZE][MAX_CODE_LENGTH];

    for (int i = 0; i < ALPHABET_SIZE; i++) {
        codes[i][0] = '\0';
    }

    char current_code[MAX_CODE_LENGTH];

    generate_codes(
        root,
        current_code,
        0,
        codes
    );

    printf("\nHuffman code table:\n");
    printf("Base\tFrequency\tCode\n");

    for (int i = 0; i < ALPHABET_SIZE; i++) {
        printf(
            "%c\t%d\t\t%s\n",
            BASES[i],
            frequencies[i],
            codes[i]
        );
    }


    long encoded_length;

    char *encoded =
        encode_sequence(
            sequence,
            codes,
            &encoded_length
        );

    if (encoded == NULL) {
        printf("Encoding failed.\n");

        free_tree(root);
        free(sequence);

        return;
    }

    printf("\nEncoding completed.\n");
    printf("Encoded length: %ld bits\n", encoded_length);


    double average_bits =
        (double)encoded_length / SEQUENCE_LENGTH;

    printf(
        "Average bits per base: %.6f\n",
        average_bits
    );


    char *decoded =
        decode_sequence(
            encoded,
            encoded_length,
            root
        );

    if (decoded == NULL) {
        printf("Decoding failed.\n");
    }
    else {


        if (strcmp(sequence, decoded) == 0) {
            printf("Round-trip verification: SUCCESS\n");
        }
        else {
            printf("Round-trip verification: FAILED\n");
        }
    }


    double fixed_bits_per_base = 2.0;

    printf(
        "Fixed-length representation: %.2f bits/base\n",
        fixed_bits_per_base
    );

    if (average_bits < fixed_bits_per_base) {

        double saving =
            100.0 *
            (fixed_bits_per_base - average_bits)
            / fixed_bits_per_base;

        printf(
            "Huffman saves approximately %.2f%% compared with fixed-length encoding.\n",
            saving
        );

    }
    else if (average_bits == fixed_bits_per_base) {

        printf(
            "Huffman provides no average bit saving over fixed-length encoding.\n"
        );

    }
    else {

        printf(
            "Huffman uses more average bits than fixed-length encoding.\n"
        );
    }


    free(decoded);
    free(encoded);
    free_tree(root);
    free(sequence);
}

int main(void) {

    srand((unsigned int)time(NULL));

    printf("CSE3144 Advanced Data Structures\n");
    printf("Part 1 - Scenario 14: The Genome Archive\n");

    double at_rich_probabilities[ALPHABET_SIZE] = {
        0.40,
        0.40,
        0.10,
        0.10
    };

    run_experiment(
        "Experiment 1: AT-rich composition",
        at_rich_probabilities
    );


    double uniform_probabilities[ALPHABET_SIZE] = {
        0.25,
        0.25,
        0.25,
        0.25
    };

    run_experiment(
        "Experiment 2: Uniform composition",
        uniform_probabilities
    );

    printf("\n");
    printf("CONCLUSION\n");

    printf(
        "Fixed-length DNA encoding requires 2 bits per base.\n"
    );

    printf(
        "For the AT-rich sequence, Huffman coding should\n"
        "benefit from the unequal symbol frequencies.\n"
    );

    printf(
        "For the uniform sequence, all four bases are equally\n"
        "frequent, so Huffman coding should approach 2 bits/base.\n"
    );

    printf(
        "The actual measured values above should be used in\n"
        "the report and observation section.\n"
    );

    return 0;
}