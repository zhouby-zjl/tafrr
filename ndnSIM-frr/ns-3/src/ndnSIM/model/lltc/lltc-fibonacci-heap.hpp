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


#ifndef FIBONACCI_HEAP_H_
#define FIBONACCI_HEAP_H_

#include <stdio.h>
#include <unordered_map>

namespace lltc {

typedef void* FHObjectPtr;

class FibonacciHeapKeyFunc {
public:
	virtual long getKey(FHObjectPtr nodeObject) { return 0; }
	virtual void setKey(FHObjectPtr nodeObject, long newKey) {} // returns a nodeObject
	virtual ~FibonacciHeapKeyFunc() {};
};

class FibonacciHeapNode {
public:
	FibonacciHeapNode(FHObjectPtr nodeObject, FibonacciHeapKeyFunc* keyFunc)
	{
		this->left = this;
		this->right = this;
		this->parent = NULL;
		this->degree = 0;
		this->keyFunc = keyFunc;
		this->nodeObject = nodeObject;
		this->child = NULL;
	}

	FHObjectPtr getNodeObject(){
		return nodeObject;
	}

	long getKey() {
		return keyFunc->getKey(nodeObject);
	}

	void setKey(long newKey) {
		keyFunc->setKey(nodeObject, newKey);
	}

	FibonacciHeapNode *left, *right, *child, *parent;
	int degree = 0;
	bool mark = false;

private:
	FHObjectPtr nodeObject;
	FibonacciHeapKeyFunc* keyFunc;
};

class FibonacciHeap {
public:
	FibonacciHeap(FibonacciHeapKeyFunc* keyFunc);
	void insert(FHObjectPtr nodeObject);
	void insert(FibonacciHeapNode* node);
	int size();
	void cut(FibonacciHeapNode* x, FibonacciHeapNode* y);
	void cascadingCut(FibonacciHeapNode* y);
	void updateKey(FHObjectPtr nodeObject, FHObjectPtr newNodeObject);
	void increaseKey(FibonacciHeapNode* x, long k);
	FHObjectPtr removeMaxObject();
	FibonacciHeapNode* removeMax();
	void degreewiseMerge();
	void makeChild(FibonacciHeapNode* y, FibonacciHeapNode* x);


private:
	FibonacciHeapNode* maxNode;
	int numberOfNodes;
	FibonacciHeapKeyFunc* keyFunc;
	std::unordered_map<FHObjectPtr, FibonacciHeapNode*>* nodeObjectMap;
};

}

#endif /* FIBONACCI_HEAP_H_ */
