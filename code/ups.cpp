#include "ups.h"
#include "Node.h"
#pragma warning(disable:4996)
Node* ups::find(int data) {
	Node* node = root;
	while (node) {
		if (node->data == data) {
			return node;
		}//same, find
		else if (node->data < data) {
			if (node->rightChild != NULL) {
				node = node->rightChild;
			}
			else {
				return NULL;
			}//no data
		}
		else {
			if (node->leftChild != NULL) {
				node = node->leftChild;
			}
			else {
				return NULL;
			}//no data
		}

	}
}
void ups::update(Node* node) { 
	node->setcount(node->getcount() + 1);
}

void ups::make(int data) { // structure : binary tree
	Node* node = root;
	Node* newnode = new Node(data);
	update(newnode);
	while (node) {
		if (node->data < data) {
			if (node->getRight() != NULL) {
				node = node->getRight();
			}
			else {
				node->setRight(newnode);
				return;
			}
		}
		else {
			if (node->leftChild != NULL) {
				node = node->getLeft();
			}
			else {
				node->setLeft(newnode);
				return;
			}
		}

	}
}

void ups::print(Node* node, vector<pair<int, int>>* p) {
	if (node->count < threshhold) return;
	(*p).push_back(pair<int, int>(node->data, node->count));
}

void ups::Inorder(Node* node, vector<pair<int, int>>* p) {
	if (node == NULL) {
		return;
	}
	else {
		Inorder(node->leftChild, p);
		print(node, p);
		Inorder(node->rightChild, p);
	}
}