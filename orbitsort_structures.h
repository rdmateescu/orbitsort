
#ifndef ORBITSORT_STRUCTURES_H
#define ORBITSORT_STRUCTURES_H

#include <stddef.h>

/* Integer node belonging to a blocklist. */
typedef struct IntegerNode
{
    int value;
    struct IntegerNode* previous;
    struct IntegerNode* next;
} IntegerNode;

/* Linked list containing the integer elements of one block. */
typedef struct BlockList
{
    IntegerNode* first;
    IntegerNode* last;
    size_t size;
} BlockList;

/* Vectorlist node referencing one blocklist. */
typedef struct VectorNode
{
    BlockList* block;
    struct VectorNode* previous;
    struct VectorNode* next;
} VectorNode;

/* Doubly linked list containing the active blocks. */
typedef struct VectorList
{
    VectorNode* first;
    size_t size;
} VectorList;

/* One pair of vectorlist nodes for an OrbitSort phase. */
typedef struct PairEntry
{
    VectorNode* first;
    VectorNode* second;
    int marker;
} PairEntry;

/* Fixed-capacity array containing the pairs for one phase. */
typedef struct PairVector
{
    PairEntry* pairs;
    size_t size;
    size_t capacity;
    size_t first_element_pair;
} PairVector;

//============= BLOCKLIST FUNCTIONS =============
//
// Functions for managing a blocklist, which stores the integer
// elements belonging to one block. The blocklist owns its
// integer nodes and maintains pointers to its first and last nodes.

int initialize_blocklist(BlockList* blocklist);
int append_integer(BlockList* blocklist, int value);
int destroy_blocklist(BlockList* blocklist);

//============= VECTORLIST FUNCTIONS =============
//
// Functions for managing the vectorlist, the outer doubly linked
// list containing the active blocks.
//
// Each VectorNode references one BlockList. The vectorlist size
// represents the number of active blocks, not the total number
// of integer elements stored in those blocks.
int append_block(VectorList* vectorlist, BlockList* blocklist); 
int initialize_vectorlist(VectorList* vectorlist);
int destroy_vectorlist(VectorList* vectorlist);
int initialize_vectorlist_from_array(
    VectorList* vectorlist,
    const int* input_array,
    size_t number_of_elements);

//============= PAIRVECTOR FUNCTIONS =============
//
// Functions for managing the pairvector, a fixed-capacity array
// containing the VectorNode pairs assigned during an OrbitSort phase.
//
// The pairvector stores references to VectorNode elements.
// It does not own or destroy those nodes.

int initialize_pairvector(PairVector* pairvector, size_t capacity);
int destroy_pairvector(PairVector* pairvector);

#endif /* ORBITSORT_STRUCTURES_H */
