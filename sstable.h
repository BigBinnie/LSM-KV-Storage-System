#include<iostream>
#include<filesystem>
#include<fstream>
#include<cstdio>
#include<stdlib.h>
#include<stdio.h>
#include<string>
#include<list>
#include"node.h"

using namespace std;
#define MaxSize 2097152


class file
{
public:
    int level_num;
    int file_num;
    int offset;
    key Max;
    key Min;
    string file_name;
    vector<offsetNode> navi; 
    file()
    {}
    file(string file_name,vector<offsetNode> &nav,int offset)
    :offset(offset),file_name(file_name)
    {
        navi=nav;
        Min=nav[0].K;
        Max=nav[nav.size()-2].K;
        cout<<file_name<<""<<Max<<" "<<Min<<endl;
    }
    file(string file_name,vector<offsetNode> &nav,string &data)
    :file_name(file_name)
    {      
        navi=nav;
        Min=nav[0].K;
        Max=nav[nav.size()-2].K;
        ofstream ofile(file_name, ios::binary | ios::out);
        ofile.write(data.c_str(),data.length());
        offset = ofile.tellp();
        ofile.close();
    }

    file(int level,int file,vector<offsetNode> &nav,string &data,string &n)
    :level_num(level),file_num(file)
    {
        navi = nav;
        navi=nav;
        Min=nav[0].K;
        Max=nav[nav.size()-2].K;
       
        clock_t time= clock();
        file_name = "data/Level "+to_string(level_num)+"/Data"+to_string(time)+".dat";
        ofstream ofile(file_name, ios::binary | ios::out);
        ofile.write(data.c_str(),data.length());
        offset = ofile.tellp();
        ofile.write(n.c_str(),n.length());
        string o=to_string(offset);
        while(o.length()<8)o="0"+o;
        ofile.write(o.c_str(),o.length());
        ofile.close();
    }
    ~file()
    {}

};


class sstable
{
    offsetNode* binary_search(vector<offsetNode>&navi,int low,int high,key k);
    string transfer_nevi(vector<offsetNode>&navi);
    file* MergeSort(file &arr1,file &arr2);
    file* merge_unordered_file(vector<file> &f);
    file* merge_ordered_file(vector<file> &f);
    void divide(int level,file *tmp);
public:
    int curdepth = 0;
    int times=0;
    vector<int> size;
    vector<list<file>> filelist;
    sstable();
    ~sstable();
    void addNewFile(string &data,vector<offsetNode> &navi);
    value search(key k);
    void compaction(int level); 
};
