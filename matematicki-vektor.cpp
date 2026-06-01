#include <bits/stdc++.h>

using namespace std;

class vektor{

    int x;
    int y;
    int z;
public:
    double len()const{

        return sqrt(pow(x,2)+pow(y,2)+pow(z,2));
    }
    vektor(){x=0,y=0,z=0;}
    ~vektor(){}
    vektor(const vektor& other){
        x=other.x;
        y=other.y;
        z=other.z;
    }
    vektor& operator=(const vektor& other){

        x=other.x;
        y=other.y;
        z=other.z;
        return *this;
    }
    vektor operator+(const vektor& other)const{

        vektor temp(other);
        temp.x+=x;
        temp.y+=y;
        temp.z+=z;
        return temp;
    }
    vektor operator-(const vektor& other)const{

        vektor temp(*this);
        temp.x-=other.x;
        temp.y-=other.y;
        temp.z-=other.z;
        return temp;
    }
    vektor operator*(const int n)const{

        vektor temp(*this);
        temp.x*=n;
        temp.y*=n;
        temp.z*=n;
        return temp;
    }
    bool operator==(const vektor& other)const{

        if(len()==other.len()){

            return true;
        }
        return false;
    }
    bool operator>(const vektor& other)const{

        if(len()>other.len()){

            return true;
        }
        return false;
    }
    bool operator<(const vektor& other)const{

        if(len()<other.len()){

            return true;
        }
        return false;
    }
    bool operator<=(const vektor& other)const{

        if(len()<=other.len()){

            return true;
        }
        return false;
    }
    bool operator>=(const vektor& other)const{

        if(len()>=other.len()){

            return true;
        }
        return false;
    }
    bool operator!=(const vektor& other)const{

        if(len()!=other.len()){

            return true;
        }
        return false;
    }
    vektor& operator++(){

        ++x;
        ++y;
        ++z;
        return *this;
    }
    vektor operator++(int){

        vektor temp(*this);
        ++x;
        ++y;
        ++z;
        return temp;
    }
    vektor& operator--(){

        --x;
        --y;
        --z;
        return *this;
    }
    vektor operator--(int){

        vektor temp(*this);
        --x;
        --y;
        --z;
        return temp;
    }
    vektor& operator+(){

        return *this;
    }
    vektor operator-(){

        vektor n(*this);
        n.x*=-1;
        n.y*=-1;
        n.z*=-1;
        return n;
    }
    friend istream& operator>>(istream& vnesi,vektor& vec);
    friend ostream& operator<<(ostream& kazi,const vektor& vec);
};
istream& operator>>(istream& vnesi,vektor& vec){

    int a;
    vnesi>>a;
    vec.x=a;
    vnesi>>a;
    vec.y=a;
    vnesi>>a;
    vec.z=a;
    return vnesi;
}
ostream& operator<<(ostream& kazi,const vektor& vec){

    kazi<<"["<<vec.x<<","<<vec.y<<","<<vec.z<<"]";
    return kazi;
}
int main() {



    return 0;
}
