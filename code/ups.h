#pragma once
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string.h>
#include <list>
#include <cstring>
#include <stdlib.h>
#include <vector>
#include <fstream>
#include "Node.h"
#pragma warning(disable:4996)
using namespace std;
class ups
{
private:
    Node* root;
    const int threshhold = 0;
public:
    ups() : root(NULL) {}
    void setRoot(Node* node) { root = node; }
    Node* getRoot() { return root; }

    Node* find(int data); //find node which data is same with parameter 'data'
    void update(Node* node); //when emerge ups, update the ups's node count +1  
    void make(int data);// make new node 
    void print(Node* node, vector<pair<int, int>>* p); //push data(node->data(ups_num), node->count) to vector p
    void Inorder(Node* node, vector<pair<int, int>>* p); 
};

