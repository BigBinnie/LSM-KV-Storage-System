#pragma once
#include <iostream>
#include <string>
#include<vector>
#include<random>
#include<ctime>
#include"node.h"
using namespace std;

struct Node
{
    key K;
    value V;
    bool isDelete = 0;
    clock_t time;
    vector<Node*> next;
    int size = 0;

    Node(key k,value v,int level,bool isdelete):K(k),V(v),isDelete(isdelete)
    {
        for(int i = 0; i<level; i++)
            next.push_back(nullptr);
        time = clock();
        size = 35+v.length();
    }
};

class SkipList
{
    Node *head;
    Node *tail;
    int maxlevel;
    int size;
    int randomLevel();

public:
    SkipList(int maxlevel, value init);
    ~SkipList();
    int getSize();
    void insert(key k,value v,bool isdelete);
    Node* search(key k);
    bool del(key k);
    void tranfersstable(vector<offsetNode>& navi,string &data);  
};