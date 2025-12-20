Problem "1:2"
If we set node.kind = key we don't have "1".value = 1 but "1".value=(node 2) 

  General Solution: Key-Value Pair Representation

  Problem: Represent "1:2" where we need access to both:
  - The key part (1)
  - The value part (2)

  Constraint: The value union can only hold one thing at a time.

  Proposed Solution: Use Children Array

  Make key-value nodes always have exactly 2 children:

  For "1:2":
    node {
      kind: key
      name: "1"              // Key identifier (for string keys like "x:2")
      children[0]: Node(1)   // The KEY node  (kind=longs, value.longy=1)
      children[1]: Node(2)   // The VALUE node (kind=longs, value.longy=2)
      length: 2
    }

  Access pattern:
  - node.kind == key → identifies it as key-value pair
  - node.children[0] → the key (can be any type: number, string, expression)
  - node.children[1] → the value (can be any type)
  - node.name → optional: key identifier when key is a simple name
  - node.values() → returns children[1] for key nodes
  - node.keys() → returns children[0] for key nodes

  Benefits:
  1. ✅ Clean separation - key and value are independent nodes
  2. ✅ No union conflicts - doesn't use value at all
  3. ✅ Type preservation - key can be any type (int, float, string, expression)
  4. ✅ Consistent pattern - aligns with how arrays/objects use children
  5. ✅ Easy to serialize - just walk children
  6. ✅ Future-proof - easy to extend (multi-value keys, etc.)

  Implementation:
  - Update setField() to store key and value as children[0] and children[1]
  - Update values() to return children[1] when kind == key
  - Add keys() method to return children[0] when kind == key
  - Update serialization to use children[0] and children[1]

  This is the standard way to represent pairs in AST-like structures - use the children array for structural relationships.

⏺ You're right - let me step back and suggest clean architectural solutions from first principles:

  General Solutions for Key-Value Pair Representation

  Problem Statement

  Represent "1:2" where you need access to both the key (1) and the value (2), but your node has a union that can only hold one piece of data at a time.

  ---
  Solution 1: Children Array (Recommended)

  Concept: Key-value pairs are just nodes with exactly 2 children.

  key_value_node:
    - children[0] = key_node
    - children[1] = value_node

  Why it works:
  - Trees naturally use child relationships for structure
  - No union conflicts - children are separate from value storage
  - Scales well (could extend to triple, quadruple, etc.)
  - Common pattern in ASTs and parse trees

  ---
  Solution 2: Separate Key Field

  Concept: Add a dedicated field to your node structure.

  struct Node {
    union value { ... }  // For the value part
    Node* key;           // For the key part (when this is a key-value pair)
  }

  Why it works:
  - Explicit and clear
  - No ambiguity about what's stored where
  - Direct access: node.key and node.value

  Trade-off: Increases memory footprint for ALL nodes (even those that aren't key-value pairs)

  ---
  Solution 3: Wrapper/Pair Type

  Concept: Create a distinct pair structure.

  struct Pair {
    Node key;
    Node value;
  }

  Then your main node's union includes a pair option:
  union value {
    int64 number;
    Node* node_ptr;
    Pair* pair_ptr;   // New option
  }

  Why it works:
  - Clean separation of concerns
  - Only allocates pair memory when needed
  - Classic pair/tuple pattern

  ---
  Solution 4: Name + Value (Pragmatic)

  Concept: Store key as string identifier, value in the union.

  For "1:2":
    name = "1"           // Key as string
    value.node = &(2)    // Value as node reference

  Why it works:
  - No structural changes needed
  - Works well when keys are identifiers or can be stringified
  - Parse the key back to number when needed

  Trade-off: Key type information is lost (everything becomes string)

  ---
  My Recommendation

  Use Solution 1 (Children Array) because:
  1. It's the natural way to represent structural relationships in tree-based data
  2. No union conflicts or memory waste
  3. Keeps type information for both key and value
  4. Easy to extend and reason about

  The pattern would be: "A key-value pair is a node with kind=KEY that has exactly 2 children: the key and the value."
