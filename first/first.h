typedef struct Node {
    long int setIndex;
    long int tag;
    long int wasPrefetch;
    struct Node* next;
} Node;

typedef struct HashTable {
    long int capacity;
    Node** array;
} HashTable;
