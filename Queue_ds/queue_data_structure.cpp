#include <iostream>

#define SIZE 10  // Default queue size

using namespace std;

// Queue class definition
class queue {
    int *arr;      // Dynamic array to hold queue elements
    int capacity;  // Maximum capacity of the queue
    int start;     // Index of the front element
    int end;       // Index of the last element
    int size;      // Current queue size
    
    friend class q_iterator;  // Allow q_iterator class to access private members

public:
    void push(int);   // Function to add element to the queue
    void pop();       // Function to remove front element
    queue(int size = SIZE); // Constructor with default size
    ~queue();        // Destructor
    int full();      // Function to check if queue is full
    int empty();     // Function to check if queue is empty
    int peek();      // Function to get front element
    void iterator(); // Placeholder function (not implemented)
};

// Queue constructor
queue::queue(int size) {
    this->size = size;
    arr = new int[size]; // Dynamically allocate memory for queue
    capacity = size;
    start = 0; // Initialize front index
    end = -1;  // Initialize rear index
}

// Queue destructor
queue::~queue() {
    delete[] arr; // Free allocated memory
}

// Function to add an element to the queue
void queue::push(int element) {
    if (full()) {  // Check if queue is full
        cout << "Overflow, program terminated \n" << endl;
        exit(EXIT_FAILURE);
    }
    arr[++end] = element; // Insert element and increment end
}

// Function to check if the queue is full
int queue::full() {
    return (end == size - 1);
}

// Function to remove front element from the queue
void queue::pop() {
    if (empty()) {  // Check if queue is empty
        cout << "Underflow, program terminated \n" << endl;
        exit(EXIT_FAILURE);
    }
    cout << "Removing " << peek() << endl; // Display removed element
    start++; // Increment start index to remove the element
}

// Function to get front element of the queue
int queue::peek() {
    if (!empty()) {
        return arr[start];
    } else {
        return 0; // Return 0 if queue is empty
    }
}

// Function to check if queue is empty
int queue::empty() {
    return (start > end);
}

// Iterator class definition
class q_iterator {
    int *it; // Pointer for iteration

public:
    q_iterator();  // Constructor
    ~q_iterator(); // Destructor
    int* begin_q(const queue &); // Get iterator to front element
    int* end_q(const queue &);   // Get iterator to last element
    const int* operator=(const int *n);
    const int* operator<(const int *n);
    const int* operator++();
};

// Iterator constructor
q_iterator::q_iterator() {
    it = nullptr; 
}

// Iterator destructor
q_iterator::~q_iterator() {}

// Get iterator to front element
int* q_iterator::begin_q(const queue &q) {
    return &q.arr[q.start];
}

// Get iterator to last element
int* q_iterator::end_q(const queue &q) {
    return &q.arr[q.end];
}

// Operator overload for comparison (not fully implemented)
const int* q_iterator::operator<(const int *n) {
    return n;  
}

// Main function to test queue operations
int main(int argc, char *argv[]) {
    queue Q{5}; // Create queue with size 5

    // Push elements into the queue
    Q.push(10);
    Q.push(20);
    Q.push(30);
    Q.push(40);
    Q.push(50);

    // Pop and display elements
    Q.pop();
    cout << Q.peek() << endl; // Should print 20
    
    Q.push(60); // Add another element
    Q.pop();

    cout << Q.peek() << endl; // Should print 30

    // Using iterator
    q_iterator it;
    cout << "Value is " << *it.begin_q(Q) << endl; // Print first element
}
