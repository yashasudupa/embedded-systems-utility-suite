#include <iostream>

#define SIZE 10

using namespace std;

class queue{

    int *arr;
    int capacity;
    int start;
    int end;
    int size;
    friend class q_iterator;

    public:
        void push(int);
        void pop();
        queue(int size = SIZE);
        ~queue();
        int full();
        int empty();
        int peek();
        void iterator();
};

queue::queue(int size){
    this->size = size;
    arr = new int[size];
    capacity = size;
    start = 0;
    end = -1;
}

queue::~queue(){
    delete[] arr;
}

void queue::push(int element){

    if(full()){
        cout << "Overflow program terminated \n" << endl;
        exit(EXIT_FAILURE);
    }

    arr[++end] = element;
}

int queue::full(){

    if(end == size){
        return 1;
    }

    return 0;
}

void queue::pop(){

    if(empty()){
        cout << "Overflow program terminated \n" << endl;
        exit(EXIT_FAILURE);
    }

    cout << "Removing " << peek() << endl;
    start++;
}

int queue::peek(){

    if(!empty()){
        return arr[start];
    }
    else{
        return 0;
    }

}

queue::empty(){

    if(end == -1){
        return 1;
    }
        return 0;
    
}


class q_iterator {

    int *it;

    public:
        q_iterator();
        ~q_iterator();
        int* begin_q(const queue &);
        int* end_q(const queue &);
        const int* operator=(const int *n);
        const int* operator<(const int *n);
        const int* operator++();
};

q_iterator::q_iterator() 
    {
        it = nullptr; 
    }

q_iterator::~q_iterator() {};

int* q_iterator::begin_q(const queue &q){

    return &q.arr[q.start];
}

int* q_iterator::end_q(const queue &q){

    return &q.arr[q.end];
}

const int* q_iterator::operator<(const int *n){
    return n;  
}


int main(int argc, char *argv[]){

    queue Q{5};

    Q.push(10);
    Q.push(20);
    Q.push(30);
    Q.push(40);
    Q.push(50);

    Q.pop();

    cout << Q.peek() << endl;
    
    Q.push(60);
    Q.pop();

    cout << Q.peek() << endl;

    q_iterator it;

    cout << "Value is" << *it.begin_q(Q) << endl;

}