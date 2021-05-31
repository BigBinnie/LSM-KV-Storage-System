#pragma once
#include<iostream>
#include <string>
#include<vector>
#include<ctime>

#define MaxInt numeric_limits<uint64_t>::max()
#define MinInt numeric_limits<uint64_t>::min()

using namespace std;
typedef uint64_t key;
typedef string value;
typedef int offset;

struct offsetNode
{
    key K;
    offset O;
    bool isdelete = 0;
    clock_t time=0;
    offsetNode(){}
    offsetNode(key k,offset o,bool b,clock_t t):K(k),O(o),isdelete(b),time(t)
    {}
};