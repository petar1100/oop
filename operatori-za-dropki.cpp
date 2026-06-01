#include<bits/stdc++.h>
using namespace std;

// +, -, *, /, ==, <, >
class Frac {
    int n;
    int d;
public:
    int N () const { return this->n; }
    int D () const { return this->d; }
    void setN(int x) { this->n = x; }
    void setD(int x) { this->d = x; }

    Frac(const int n, const int d=1) {
        this->n = n;
        this->d = d;
    }

    // Copy C-tor
    Frac (const Frac& orig) {
        this->n = orig.n;
        this->d = orig.d;
    }

    Frac& operator=(const Frac& orig) {
        if (this!=&orig) { // No Self-assignment
            this->n = orig.n;
            this->d = orig.d;
        }
        return *this;
    }



    void print() {
        cout<<this->n <<"/" << this->d;
    }

    Frac operator*(const Frac& b) const {
        Frac c(*this);
        c.setN(this->N() * b.N());
        c.setD(this->D() * b.D());
        if(c.N()%c.D()==0){

            c.setN(c.N()/c.D());
            c.setD(1);
        }
        return c;
    }
    Frac operator+(const Frac& b)const{

        Frac c(*this);
        if (this->D()%b.D()==0){

            c.setN(this->N()+(b.N()*(this->D()/b.D())));
            c.setD(this->D());
        }
        else if (b.D()%this->D()==0){

            c.setN(this->N()*(b.D()/this->D())+b.N());
            c.setD(b.D());
        }
        else{

            c.setN(this->N()*b.D()+b.N()*this->D());
            c.setD(this->D()*b.D());
        }
        if(c.N()%c.D()==0){

            c.setN(c.N()/c.D());
            c.setD(1);
        }
        return c;
    }
    Frac operator-(const Frac& b)const{

        Frac c(*this);
        if (this->D()%b.D()==0){

            c.setN(this->N()-(b.N()*(this->D()/b.D())));
            c.setD(this->D());
        }
        else if (b.D()%this->D()==0){

            c.setN(this->N()*(b.D()/this->D())-b.N());
            c.setD(b.D());
        }
        else{

            c.setN(this->N()*b.D()-b.N()*this->D());
            c.setD(this->D()*b.D());
        }
        if(c.N()%c.D()==0){

            c.setN(c.N()/c.D());
            c.setD(1);
        }
        return c;
    }
    Frac operator/(const Frac& b)const{

        Frac c(*this);
        c.setN(this->N()*b.D());
        c.setD(b.N()*this->D());
        if(c.N()%c.D()==0){

            c.setN(c.N()/c.D());
            c.setD(1);
        }
        return c;
    }
    bool operator==(const Frac& b)const{

        float a=(float)this->N()/(float)this->D();
        float c=(float)b.N()/(float)b.D();
        if(a==c){

            return true;
        }
        else{

            return false;
        }
    }
    bool operator>(const Frac& b)const{

        float a=(float)this->N()/(float)this->D();
        float c=(float)b.N()/(float)b.D();
        if(a>c){

            return true;
        }
        else{

            return false;
        }
    }
    bool operator<(const Frac& b)const{

        float a=(float)this->N()/(float)this->D();
        float c=(float)b.N()/(float)b.D();
        if(a<c){

            return true;
        }
        else{

            return false;
        }
    }


    ~Frac() {}

};

ostream& operator<<(ostream& o, const Frac& f) {
    o<<f.N() <<"/" << f.D();
    return o;
}

// Frac operator*(const Frac& a, const Frac& b) {
//     Frac c(a);
//     c.setN(a.N() * b.N());
//     c.setD(a.D() * b.D());
//     return c;
// }

int main() {
    Frac a(2,15);
    Frac b(4,5);
    Frac c=a/b;
    if(a>b){

        cout<<c;
    }

    // Global function operator* (Frac, Frac)
    // Member Function operator* (Frac)
    // a * b
    // operator*(a,b)
    // a.operator*(b)

    //.c.print();

}
