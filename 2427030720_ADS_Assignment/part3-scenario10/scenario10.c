
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>

#define RANDOM_OPERATIONS 100000
#define MAX_REFERENCE_SIZE (RANDOM_OPERATIONS + 10)
#define INITIAL_KEY_RANGE 1000000000

typedef struct PairingNode {
    int key;
    struct PairingNode *firstChild;
    struct PairingNode *nextSibling;
} PairingNode;

typedef struct {
    PairingNode *root;
    int size;
} PairingHeap;

typedef struct {
    int key;
    PairingNode *handle;
} ReferenceItem;

static long long totalExtractLinks = 0;


static PairingNode *create_node(int key) {
    PairingNode *node = (PairingNode *)malloc(sizeof(PairingNode));

    if (node == NULL) {
        fprintf(stderr, "Memory allocation failed.\n");
        exit(EXIT_FAILURE);
    }

    node->key = key;
    node->firstChild = NULL;
    node->nextSibling = NULL;

    return node;
}

static void heap_init(PairingHeap *heap) {
    heap->root = NULL;
    heap->size = 0;
}

static PairingNode *link_roots(PairingNode *a, PairingNode *b) {
    if (a == NULL) return b;
    if (b == NULL) return a;

    if (b->key < a->key) {
        PairingNode *tmp = a;
        a = b;
        b = tmp;
    }

    b->nextSibling = a->firstChild;
    a->firstChild = b;

    return a;
}


static PairingNode *heap_insert(PairingHeap *heap, int key) {
    PairingNode *node = create_node(key);

    heap->root = link_roots(heap->root, node);
    heap->size++;

    return node;
}


static int heap_find_min(const PairingHeap *heap, int *result) {
    if (heap->root == NULL) {
        return 0;
    }

    *result = heap->root->key;
    return 1;
}


static PairingNode *combine_children(PairingNode *firstChild) {
    if (firstChild == NULL) {
        return NULL;
    }

    PairingNode *pairs = NULL;
    PairingNode *current = firstChild;


    while (current != NULL) {
        PairingNode *a = current;
        PairingNode *b = a->nextSibling;
        PairingNode *remaining;

        a->nextSibling = NULL;

        if (b != NULL) {
            remaining = b->nextSibling;
            b->nextSibling = NULL;
        } else {
            remaining = NULL;
        }

        PairingNode *merged;

        if (b != NULL) {
            merged = link_roots(a, b);
            totalExtractLinks++;
        } else {
            merged = a;
        }


        merged->nextSibling = pairs;
        pairs = merged;

        current = remaining;
    }

    PairingNode *result = NULL;
    current = pairs;

    while (current != NULL) {
        PairingNode *next = current->nextSibling;
        current->nextSibling = NULL;

        if (result != NULL) {
            result = link_roots(current, result);
            totalExtractLinks++;
        } else {
            result = current;
        }

        current = next;
    }

    return result;
}

static int heap_extract_min(PairingHeap *heap, int *result) {
    if (heap->root == NULL) {
        return 0;
    }

    PairingNode *oldRoot = heap->root;
    *result = oldRoot->key;

    PairingNode *children = oldRoot->firstChild;

    oldRoot->firstChild = NULL;
    oldRoot->nextSibling = NULL;

    heap->root = combine_children(children);
    heap->size--;

    free(oldRoot);
    return 1;
}


static int detach_node(PairingHeap *heap, PairingNode *target) {
    if (heap->root == NULL || target == NULL || target == heap->root) {
        return 0;
    }

    PairingNode *parent = heap->root;
    PairingNode *stack[MAX_REFERENCE_SIZE];
    int top = 0;

    PairingNode *foundParent = NULL;

    stack[top++] = parent;

    while (top > 0 && foundParent == NULL) {
        PairingNode *node = stack[--top];

        PairingNode *child = node->firstChild;

        while (child != NULL) {
            if (child == target) {
                foundParent = node;
                break;
            }

            if (child->firstChild != NULL && top < MAX_REFERENCE_SIZE) {
                stack[top++] = child;
            }

            child = child->nextSibling;
        }
    }

    if (foundParent == NULL) {
        return 0;
    }

    PairingNode *child = foundParent->firstChild;

    if (child == target) {
        foundParent->firstChild = target->nextSibling;
    } else {
        while (child != NULL && child->nextSibling != target) {
            child = child->nextSibling;
        }

        if (child == NULL) {
            return 0;
        }

        child->nextSibling = target->nextSibling;
    }

    target->nextSibling = NULL;
    return 1;
}

static int heap_decrease_key(PairingHeap *heap,
                             PairingNode *node,
                             int newKey) {
    if (node == NULL || newKey >= node->key) {
        return 0;
    }

    node->key = newKey;

    if (node == heap->root) {
        return 1;
    }

    if (!detach_node(heap, node)) {
        return 0;
    }

    heap->root = link_roots(heap->root, node);
    return 1;
}

typedef struct {
    int key;
    PairingNode *handle;
} RefItem;

typedef struct {
    RefItem items[MAX_REFERENCE_SIZE];
    int size;
} ReferenceQueue;

static void ref_init(ReferenceQueue *ref) {
    ref->size = 0;
}

static void ref_insert(ReferenceQueue *ref, int key, PairingNode *handle) {
    if (ref->size >= MAX_REFERENCE_SIZE) {
        fprintf(stderr, "Reference queue capacity exceeded.\n");
        exit(EXIT_FAILURE);
    }

    int i = ref->size++;

    while (i > 0 && ref->items[i - 1].key > key) {
        ref->items[i] = ref->items[i - 1];
        i--;
    }

    ref->items[i].key = key;
    ref->items[i].handle = handle;
}

static int ref_find_min(const ReferenceQueue *ref, int *key) {
    if (ref->size == 0) {
        return 0;
    }

    *key = ref->items[0].key;
    return 1;
}

static int ref_extract_min(ReferenceQueue *ref, int *key) {
    if (ref->size == 0) {
        return 0;
    }

    *key = ref->items[0].key;

    for (int i = 1; i < ref->size; i++) {
        ref->items[i - 1] = ref->items[i];
    }

    ref->size--;
    return 1;
}

static int ref_decrease_key(ReferenceQueue *ref,
                            PairingNode *handle,
                            int newKey) {
    for (int i = 0; i < ref->size; i++) {
        if (ref->items[i].handle == handle) {
            if (newKey >= ref->items[i].key) {
                return 0;
            }

            ref->items[i].key = newKey;

            RefItem item = ref->items[i];

            for (int j = i; j > 0 && ref->items[j - 1].key > item.key; j--) {
                ref->items[j] = ref->items[j - 1];
                ref->items[j - 1] = item;
            }

            return 1;
        }
    }

    return 0;
}

static void free_tree(PairingNode *node) {
    if (node == NULL) return;

    PairingNode *child = node->firstChild;

    while (child != NULL) {
        PairingNode *next = child->nextSibling;
        free_tree(child);
        child = next;
    }

    free(node);
}

static void heap_clear(PairingHeap *heap) {
    free_tree(heap->root);
    heap->root = NULL;
    heap->size = 0;
}

int main(void) {
    PairingHeap heap;
    ReferenceQueue ref;

    heap_init(&heap);
    ref_init(&ref);

    srand((unsigned int)time(NULL));

    int nextUniqueKey = 1;
    int successfulInserts = 0;
    int successfulExtracts = 0;
    int successfulDecreases = 0;
    int findMinChecks = 0;
    int failedChecks = 0;

    printf("CSE3144 Advanced Data Structures\n");
    printf("Part 3 - Scenario 10: Rover Firmware Queue\n\n");

    printf("Data structure: Pairing Heap\n");
    printf("Node fields: key, firstChild, nextSibling\n");
    printf("Random operations: %d\n\n", RANDOM_OPERATIONS);

    for (int op = 0; op < RANDOM_OPERATIONS; op++) {
        int choice = rand() % 100;

        if (heap.size == 0 || (choice < 40 && heap.size < 20000)) {
            int key = nextUniqueKey++;

            PairingNode *handle = heap_insert(&heap, key);
            ref_insert(&ref, key, handle);
            successfulInserts++;
        }
        else if (choice < 60) {
            int heapMin, refMin;
            int a = heap_find_min(&heap, &heapMin);
            int b = ref_find_min(&ref, &refMin);

            findMinChecks++;

            if (a != b || (a && heapMin != refMin)) {
                failedChecks++;
                printf("ERROR: find-min mismatch at operation %d.\n", op);
                break;
            }
        }
        else if (choice < 80) {
            int heapMin, refMin;
            int a = heap_extract_min(&heap, &heapMin);
            int b = ref_extract_min(&ref, &refMin);

            if (a != b || (a && heapMin != refMin)) {
                failedChecks++;
                printf("ERROR: extract-min mismatch at operation %d.\n", op);
                break;
            }

            if (a) successfulExtracts++;
        }
        else {
 
            if (ref.size > 0) {
                int idx = rand() % ref.size;
                PairingNode *handle = ref.items[idx].handle;
                int newKey = -nextUniqueKey++;

                int a = heap_decrease_key(&heap, handle, newKey);
                int b = ref_decrease_key(&ref, handle, newKey);

                if (a != b) {
                    failedChecks++;
                    printf("ERROR: decrease-key mismatch at operation %d.\n", op);
                    break;
                }

                if (a) successfulDecreases++;
            }
        }

        if (heap.size != ref.size) {
            failedChecks++;
            printf("ERROR: size mismatch at operation %d.\n", op);
            break;
        }
    }


    while (failedChecks == 0 && heap.size > 0) {
        int heapMin, refMin;

        int a = heap_extract_min(&heap, &heapMin);
        int b = ref_extract_min(&ref, &refMin);

        if (a != b || (a && heapMin != refMin)) {
            failedChecks++;
            printf("ERROR: final drain mismatch.\n");
            break;
        }

        if (a) successfulExtracts++;
    }

    printf("Randomized Test Summary\n");

    printf("Operations attempted: %d\n", RANDOM_OPERATIONS);
    printf("Successful inserts: %d\n", successfulInserts);
    printf("Successful find-min checks: %d\n", findMinChecks);
    printf("Successful decrease-key operations: %d\n", successfulDecreases);
    printf("Successful extract-min operations: %d\n", successfulExtracts);
    printf("Total extract-min consolidation links: %lld\n", totalExtractLinks);
    printf("Reference comparison failures: %d\n", failedChecks);

    if (failedChecks == 0) {
        printf("Validation: SUCCESS\n");
    } else {
        printf("Validation: FAILED\n");
    }

    printf("\nComplexity note:\n");
    printf("- Insert: O(1) actual time.\n");
    printf("- Meld: O(1) actual time.\n");
    printf("- Find-min: O(1) actual time.\n");
    printf("- Extract-min: O(log n) amortized in standard pairing-heap analysis.\n");
    printf("- Decrease-key: O(n) worst case here because parent pointers are forbidden.\n");
    printf("  Do not claim the Fibonacci-heap O(1) amortized decrease-key bound.\n");

    heap_clear(&heap);
    return failedChecks == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}