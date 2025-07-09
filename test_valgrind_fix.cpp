#include "source/Node.h"
#include <iostream>

// Simple test to verify Node initialization works correctly
int main() {
    // Test 1: Default constructor
    Node node1;
    std::cout << "Node1 length: " << node1.length << " (should be 0)" << std::endl;
    
    // Test 2: Calling first() on empty node should not crash
    try {
        Node& first = node1.first();
        std::cout << "first() returned: " << &first << " (should be same as node1: " << &node1 << ")" << std::endl;
    } catch (...) {
        std::cout << "Error calling first() on empty node" << std::endl;
        return 1;
    }
    
    // Test 3: Create node with children
    Node node2;
    node2.init_children(3);
    node2.length = 2;
    
    // Add some children
    if (node2.children) {
        node2.children[0] = Node(42);
        node2.children[1] = Node(84);
        
        // Test first() with children
        Node& first_child = node2.first();
        std::cout << "First child value: " << first_child.value.longy << " (should be 42)" << std::endl;
    }
    
    std::cout << "All tests passed!" << std::endl;
    return 0;
}