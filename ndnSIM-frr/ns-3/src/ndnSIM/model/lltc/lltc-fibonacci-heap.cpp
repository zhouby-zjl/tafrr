/*
 * This work is licensed under CC BY-NC-SA 4.0
 * (https://creativecommons.org/licenses/by-nc-sa/4.0/).
 * Copyright (c) 2021 Boyang Zhou
 *
 * This file is a part of "Disruption Resilient Transport Protocol"
 * (https://github.com/zhouby-zjl/drtp/).
 * Written by Boyang Zhou (zhouby@zhejianglab.com)
 *
 * This software is protected by the patents numbered with PCT/CN2021/075891,
 * ZL202110344405.7 and ZL202110144836.9, as well as the software copyrights
 * numbered with 2020SR1875227 and 2020SR1875228.
 */

#include "lltc-fibonacci-heap.hpp"

#include <string>

using namespace std;
using namespace __gnu_cxx;

using namespace lltc;

FibonacciHeap::FibonacciHeap(FibonacciHeapKeyFunc* func) {
	this->keyFunc = func;
	numberOfNodes = 0;
	maxNode = NULL;
	nodeObjectMap = new unordered_map<FHObjectPtr, FibonacciHeapNode*>();
}

void FibonacciHeap::insert(FHObjectPtr nodeObject) {
	FibonacciHeapNode* node = new FibonacciHeapNode(nodeObject, keyFunc);
	insert(node);
	(*nodeObjectMap)[nodeObject] = node;
}

void FibonacciHeap::insert(FibonacciHeapNode* node) {
	//check if max node is not null
	if (maxNode != NULL) {
		//add to the right of max node
		node->left = maxNode;
		node->right = maxNode->right;
		maxNode->right = node;

		//check if node right is not null
		if (node->right != NULL) {
			node->right->left = node;
		}
		if (node->right == NULL)
		{
			node->right = maxNode;
			maxNode->left = node;
		}
		if (node->getKey() > maxNode->getKey()) {
			maxNode = node;
		}
	} else {
		maxNode = node;

	}

	numberOfNodes++;
}

int FibonacciHeap::size() {
	return numberOfNodes;
}

void FibonacciHeap::cut(FibonacciHeapNode* x, FibonacciHeapNode* y) {
	// removes x from child of y and decreases the degree of y
	x->left->right = x->right;
	x->right->left = x->left;
	y->degree--;

	// reset y.child if necessary
	if (y->child == x) {
		y->child = x->right;
	}

	if (y->degree == 0) {
		y->child = NULL;
	}

	// add x to root list of heap
	x->left = maxNode;
	x->right = maxNode->right;
	maxNode->right = x;
	x->right->left = x;

	// set parent of x to nil
	x->parent = NULL;

	// set mark to false
	x->mark = false;
}

void FibonacciHeap::cascadingCut(FibonacciHeapNode* y) {
	FibonacciHeapNode* x = y->parent;

	//if there is a parent
	if (x != NULL) {
		// if y is unmarked, set it marked
		if (!y->mark) {
			y->mark = true;
		} else {
			// it's marked, cut it from parent
			cut(y, x);

			// cut its parent as well
			cascadingCut(x);
		}
	}
}

void FibonacciHeap::updateKey(FHObjectPtr nodeObject, FHObjectPtr newNodeObject) {
	long newKey = keyFunc->getKey(newNodeObject);
	FibonacciHeapNode* x = (*(this->nodeObjectMap))[nodeObject];
	this->increaseKey(x, newKey);
}

void FibonacciHeap::increaseKey(FibonacciHeapNode* x, long k) {
	if (k < x->getKey()) {}

	x->setKey(k);
	FibonacciHeapNode* y = x->parent;

	if ((y != NULL) && (x->getKey() > y->getKey())) {
		cut(x, y);
		cascadingCut(y);
	}

	if (x->getKey() > maxNode->getKey()) {
		maxNode = x;
	}
}

FHObjectPtr FibonacciHeap::removeMaxObject() {
	FibonacciHeapNode* node = removeMax();
	return node->getNodeObject();
}

FibonacciHeapNode* FibonacciHeap::removeMax() {
	FibonacciHeapNode* z = maxNode;

	if (z != NULL) {
		this->nodeObjectMap->erase(z->getNodeObject());

		int numberofChildren = z->degree;
		FibonacciHeapNode* x = z->child;
		FibonacciHeapNode* tempRight;

		//while  there are childred of max
		while (numberofChildren > 0) {
			tempRight = x->right;

			// remove x from child list
			x->left->right = x->right;
			x->right->left = x->left;

			// add x to root list of heap
			x->left = maxNode;
			x->right = maxNode->right;
			maxNode->right = x;
			x->right->left = x;

			// set parent to null
			x->parent = NULL;
			x = tempRight;
			//decrease number of children of max
			numberofChildren--;

		}

		// remove z from root list of heap
		z->left->right = z->right;
		z->right->left = z->left;

		if (z == z->right) {
			maxNode = NULL;

		} else {
			maxNode = z->right;
			degreewiseMerge();
		}
		numberOfNodes--;
		return z;
	}


	return NULL;
}

void FibonacciHeap::degreewiseMerge() {
	//chosen at random, read on internet that 45 is most optimised,
	// else can be calculated using the formulae given in cormen
	int sizeofDegreeTable = 45;

	FibonacciHeapNode** degreeTable = new FibonacciHeapNode*[sizeofDegreeTable];

	// Initialize degree table
	for (int i = 0; i < sizeofDegreeTable; i++) {
		degreeTable[i] = NULL;
	}

	// Find the number of root nodes.
	int numRoots = 0;
	FibonacciHeapNode* x = maxNode;

	if (x != NULL) {
		numRoots++;
		x = x->right;

		while (x != maxNode) {
			numRoots++;
			x = x->right;
		}
	}

	// For each node in root list
	while (numRoots > 0) {

		int d = x->degree;
		FibonacciHeapNode* next = x->right;

		// check if the degree is there in degree table, if not add,if yes then combine and merge
		for (;;) {
			FibonacciHeapNode* y = degreeTable[d];
			if (y == NULL) {
				break;
			}

			//Check whos key value is greater
			if (x->getKey() < y->getKey()) {
				FibonacciHeapNode* temp = y;
				y = x;
				x = temp;
			}

			//make y the child of x as x key value is greater
			makeChild(y, x);

			//setthe degree to null as x and y are combined now
			degreeTable[d] = NULL;
			d++;
		}

		//store the new x(x+y) in the respective degree table postion
		degreeTable[d] = x;

		// Move forward through list.
		x = next;
		numRoots--;
	}

	//Deleting the max node
	maxNode = NULL;

	// combine entries of the degree table
	for (int i = 0; i < sizeofDegreeTable; i++) {
		FibonacciHeapNode* y = degreeTable[i];
		if (y == NULL) {
			continue;
		}

		//till max node is not null
		if (maxNode != NULL) {

			// First remove node from root list.
			y->left->right = y->right;
			y->right->left = y->left;

			// Now add to root list, again.
			y->left = maxNode;
			y->right = maxNode->right;
			maxNode->right = y;
			y->right->left = y;

			// Check if this is a new maximum
			if (y->getKey() > maxNode->getKey()) {
				maxNode = y;
			}
		} else {
			maxNode = y;
		}
	}
}


void FibonacciHeap::makeChild(FibonacciHeapNode* y, FibonacciHeapNode* x) {
	// remove y from root list of heap
	y->left->right = y->right;
	y->right->left = y->left;

	// make y a child of x
	y->parent = x;

	if (x->child == NULL) {
		x->child = y;
		y->right = y;
		y->left = y;
	} else {
		y->left = x->child;
		y->right = x->child->right;
		x->child->right = y;
		y->right->left = y;
	}

	// increase degree of x by 1
	x->degree++;

	// make mark of y as false
	y->mark = false;
}

