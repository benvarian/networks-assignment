#ifndef Node_h
#define Node_h

// Nodes are used to store data of any type in a list.  
struct Node
{
    // The data is stored as a void pointer - casting is required for proper access.
    void *data;
    // A pointer to the next node in the chain.
    struct Node *next;
    struct Node *previous;
};

// The constructor should be used to create nodes.
struct Node node_constructor(void *data, unsigned long size);
// The destructor should be used to destroy nodes.
void node_destructor(struct Node *node);

#endif
