#include <bits/stdc++.h>

using namespace std;

struct vec{

    size_t size;
    size_t capacity;
    int* data;
    void sizeup(){

        if(capacity==0){

            capacity=1;
            int* temp=new int[1];
            delete[] data;
            data=temp;
        }
        else{
            capacity*=2;
        }
    }
public:
    vec(){data= nullptr,capacity=0,size=0;}
    ~vec(){
        delete[] data;
    }
    vec(const vec& other){

        size=other.size;
        capacity=other.capacity;
        data =new int[capacity];
        for(int i=0;i<size;i++){

            data[i]=other.data[i];
        }
    }
    vec& operator=(const vec& other){

        delete[] data;
        size=other.size;
        capacity=other.capacity;
        data =new int[capacity];
        for(int i=0;i<size;i++){

            data[i]=other.data[i];
        }
        return *this;
    }
    void p_b(int a){

        if(size==capacity){

            sizeup();
        }
        data[size]=a;
        size++;
    }
    void pop(){

        if(size/2<=capacity){

            capacity/=2;
        }
        size--;
    }
    const int& operator[](int index)const{

        return data[index];
    }
    void clear(){

        delete[] data;
        size=0;
        capacity=0;
    }
    vec& operator+(const vec& other){

        while(capacity<size+other.size){
            sizeup();
        }
        for(int i=0;i<other.size;i++){

            p_b(other.data[i]);
        }
        return *this;
    }
    int at(const int index)const{

        if(index<size&&index>=0){

            return data[index];
        }
        else{

            return -1;
        }
    }
    int front()const{

        if(size>=1){

            return data[0];
        }
        else{

            return -1;
        }
    }
    int back(){

        if(size>=1){

            return data[size-1];
        }
        else{

            return -1;
        }
    }
    int* Data()const{

        return data;
    }
    friend ostream& operator<<(ostream& say, const vec& n);
};
ostream& operator<<(ostream& say, const vec& n){

    say<<"[";
    for(int i=0;i<n.size;i++){

        say<<n.data[i];
        if(i<n.size-1){
            say<<",";
        }
    }
    say<<"]";
    return say;
}
int main() {


    return 0;
}
