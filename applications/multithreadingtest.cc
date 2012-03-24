#include <stdafx.h>
#include <stdlib.h>
#include <windows.h>
#include <set>
#include <conio.h>
#include <iostream>
using namespace std;

int a,b,c,d;
int function1(int, int);
int function2(int);
int main(){
    cout << "Enter a:";
    cin >> a;
    cont << "Enter b;";
    cin >> b;
    function1(a, b);
    function2(c);
    return 0;
}
int function1(int a, int b){
    c = (2* (a + b)) - (3 * b);
    cout << "c= " << c << "\n";
    return c;
}

int function2(int c){
    d = (2 * c) + (c - 3);
    cout << "d=" << d << "\n";
    return d;
}
