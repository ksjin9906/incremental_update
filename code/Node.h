#pragma once
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string.h>
#include <list>
#include <cstring>
#include <stdlib.h>
#include <vector>
#pragma warning(disable:4996)
using namespace std;
class Node
{
public:
    int data; //state number of UPS 
    int count = 0; //UPS's count
    Node* leftChild = NULL;
    Node* rightChild = NULL;
    Node(int _data) { data = _data; }



    int getData() const { return data; }
    void setData(int _data) { data = _data; }
    int getcount() const { return count; } 
    void setcount(int num) { count = num; } 
    Node* getLeft() const { return leftChild; } 
    Node* getRight() const { return rightChild; } 
    void setLeft(Node* left) { this->leftChild = left; }
    void setRight(Node* right) { this->rightChild = right; } 
};

