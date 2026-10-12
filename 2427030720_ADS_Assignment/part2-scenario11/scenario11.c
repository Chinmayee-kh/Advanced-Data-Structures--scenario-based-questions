#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define ALPHABET_SIZE 26
#define MAX_NAME_LENGTH 100
#define MAX_SUGGESTIONS 5

typedef struct TrieNode {
    struct TrieNode *children[ALPHABET_SIZE];
    int isEndOfWord;
    char *stationName;
} TrieNode;

TrieNode* create_node(void) {
    TrieNode *node = (TrieNode *)malloc(sizeof(TrieNode));

    if (node == NULL) {
        printf("Memory allocation failed.\n");
        exit(1);
    }

    node->isEndOfWord = 0;
    node->stationName = NULL;

    for (int i = 0; i < ALPHABET_SIZE; i++) {
        node->children[i] = NULL;
    }

    return node;
}

int char_index(char c) {
    c = (char)toupper((unsigned char)c);

    if (c >= 'A' && c <= 'Z') {
        return c - 'A';
    }

    return -1;
}

void insert(TrieNode *root, const char *word) {
    TrieNode *current = root;

    for (int i = 0; word[i] != '\0'; i++) {
        int index = char_index(word[i]);

        if (index == -1) {
            continue;
        }

        if (current->children[index] == NULL) {
            current->children[index] = create_node();
        }

        current = current->children[index];
    }

    current->isEndOfWord = 1;

    if (current->stationName != NULL) {
        free(current->stationName);
    }

    current->stationName =
        (char *)malloc(strlen(word) + 1);

    if (current->stationName == NULL) {
        printf("Memory allocation failed.\n");
        exit(1);
    }

    strcpy(current->stationName, word);
}

TrieNode* find_prefix_node(
    TrieNode *root,
    const char *prefix
) {
    TrieNode *current = root;

    for (int i = 0; prefix[i] != '\0'; i++) {
        int index = char_index(prefix[i]);

        if (index == -1) {
            continue;
        }

        if (current->children[index] == NULL) {
            return NULL;
        }

        current = current->children[index];
    }

    return current;
}

void collect_suggestions(
    TrieNode *node,
    char suggestions[][MAX_NAME_LENGTH],
    int *count
) {
    if (node == NULL || *count >= MAX_SUGGESTIONS) {
        return;
    }

    if (node->isEndOfWord && node->stationName != NULL) {
        strcpy(
            suggestions[*count],
            node->stationName
        );

        (*count)++;

        if (*count >= MAX_SUGGESTIONS) {
            return;
        }
    }

    for (int i = 0; i < ALPHABET_SIZE; i++) {
        if (node->children[i] != NULL) {
            collect_suggestions(
                node->children[i],
                suggestions,
                count
            );

            if (*count >= MAX_SUGGESTIONS) {
                return;
            }
        }
    }
}

void suggest(
    TrieNode *root,
    const char *prefix
) {
    TrieNode *node =
        find_prefix_node(root, prefix);

    printf("\nPrefix: %s\n", prefix);
    printf("Suggestions:\n");

    if (node == NULL) {
        printf("No suggestions found.\n");
        return;
    }

    char suggestions[MAX_SUGGESTIONS][MAX_NAME_LENGTH];
    int count = 0;

    collect_suggestions(
        node,
        suggestions,
        &count
    );

    if (count == 0) {
        printf("No suggestions found.\n");
        return;
    }

    for (int i = 0; i < count; i++) {
        printf(
            "%d. %s\n",
            i + 1,
            suggestions[i]
        );
    }
}

void clean_name(
    const char *word,
    char *cleaned
) {
    int position = 0;

    for (int i = 0; word[i] != '\0'; i++) {
        int index = char_index(word[i]);

        if (index != -1) {
            cleaned[position++] =
                (char)toupper(
                    (unsigned char)word[i]
                );
        }
    }

    cleaned[position] = '\0';
}


int delete_helper(
    TrieNode *node,
    const char *word,
    int depth,
    int length
) {
    if (node == NULL) {
        return 0;
    }

    if (depth == length) {
        if (!node->isEndOfWord) {
            return 0;
        }

        node->isEndOfWord = 0;

        if (node->stationName != NULL) {
            free(node->stationName);
            node->stationName = NULL;
        }

        for (int i = 0; i < ALPHABET_SIZE; i++) {
            if (node->children[i] != NULL) {
                return 0;
            }
        }

        return 1;
    }

    int index = char_index(word[depth]);

    if (index == -1) {
        return delete_helper(
            node,
            word,
            depth + 1,
            length
        );
    }

    if (node->children[index] == NULL) {
        return 0;
    }

    int should_delete_child =
        delete_helper(
            node->children[index],
            word,
            depth + 1,
            length
        );

    if (should_delete_child) {
        free(node->children[index]);
        node->children[index] = NULL;

        if (!node->isEndOfWord) {
            for (int i = 0; i < ALPHABET_SIZE; i++) {
                if (node->children[i] != NULL) {
                    return 0;
                }
            }

            return 1;
        }
    }

    return 0;
}

void delete_station(
    TrieNode *root,
    const char *word
) {
    char cleaned[MAX_NAME_LENGTH];

    clean_name(
        word,
        cleaned
    );

    if (!find_prefix_node(root, cleaned)) {
        printf(
            "Station \"%s\" not found.\n",
            word
        );
        return;
    }

    TrieNode *node =
        find_prefix_node(root, cleaned);

    if (node == NULL || !node->isEndOfWord) {
        printf(
            "Station \"%s\" not found.\n",
            word
        );
        return;
    }

    delete_helper(
        root,
        cleaned,
        0,
        strlen(cleaned)
    );

    printf(
        "Station \"%s\" deleted successfully.\n",
        word
    );
}

void free_trie(TrieNode *node) {
    if (node == NULL) {
        return;
    }

    for (int i = 0; i < ALPHABET_SIZE; i++) {
        free_trie(node->children[i]);
    }

    if (node->stationName != NULL) {
        free(node->stationName);
    }

    free(node);
}

int search(
    TrieNode *root,
    const char *word
) {
    TrieNode *node =
        find_prefix_node(root, word);

    if (node == NULL) {
        return 0;
    }

    return node->isEndOfWord;
}

int main(void) {

    printf(
        "CSE3144 Advanced Data Structures\n"
    );

    printf(
        "Part 2 - Scenario 11: "
        "Station-Name Suggestions\n"
    );

    TrieNode *root =
        create_node();

  
    const char *stations[] = {
        "JAIPUR",
        "JAISALMER",
        "JAMMU",
        "JALANDHAR",
        "JABALPUR",
        "JAIPUR CANTT",
        "JAGDALPUR"
    };

    int station_count =
        sizeof(stations) /
        sizeof(stations[0]);

    printf(
        "\nInserting station names:\n"
    );

    for (int i = 0;
         i < station_count;
         i++) {

        insert(
            root,
            stations[i]
        );

        printf(
            "Inserted: %s\n",
            stations[i]
        );
    }


    printf(
        "Exact Search Tests\n"
    );


    printf(
        "Search JAIPUR: %s\n",
        search(root, "JAIPUR")
            ? "FOUND"
            : "NOT FOUND"
    );

    printf(
        "Search JAIPUR CANTT: %s\n",
        search(root, "JAIPUR CANTT")
            ? "FOUND"
            : "NOT FOUND"
    );

    printf(
        "Search DELHI: %s\n",
        search(root, "DELHI")
            ? "FOUND"
            : "NOT FOUND"
    );



    printf(
        "Prefix Suggestion Tests\n"
    );


    suggest(root, "JA");
    suggest(root, "JAI");
    suggest(root, "JAIP");
    suggest(root, "JAL");
    suggest(root, "X");



    printf(
        "Deletion Test\n"
    );


    delete_station(
        root,
        "JAGDALPUR"
    );

    printf(
        "Search JAGDALPUR after deletion: %s\n",
        search(root, "JAGDALPUR")
            ? "FOUND"
            : "NOT FOUND"
    );

    printf(
        "\nRe-inserting JAGDALPUR...\n"
    );

    insert(
        root,
        "JAGDALPUR"
    );

    printf(
        "Search JAGDALPUR after reinsertion: %s\n",
        search(root, "JAGDALPUR")
            ? "FOUND"
            : "NOT FOUND"
    );

    printf(
        "\nSuggestions for JA after reinsertion:\n"
    );

    suggest(
        root,
        "JA"
    );

    free_trie(root);

    printf(
        "Program completed successfully.\n"
    );



    return 0;
}