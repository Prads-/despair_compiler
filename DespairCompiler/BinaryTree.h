/*
	Written By Pradipna Nepal
	www.pradsprojects.com

	Copyright (C) 2013 Pradipna Nepal
	Please read COPYING.txt included along with this source code for more detail.
	If not included, see http://www.gnu.org/licenses/
*/

#ifndef BINARY_TREE_H
#define BINARY_TREE_H

#include <vector>

template <typename NodeType>
class TreeNode {
private:
	NodeType *parent;
	TreeNode *leftNode, *rightNode;

public:
	NodeType *getParent() {
		return parent;
	}

	TreeNode *getLeftNode() {
		return leftNode;
	}

	TreeNode *getRightNode() {
		return rightNode;
	}

	TreeNode **getLeftNodePtr() {
		return &leftNode;
	}

	TreeNode **getRightNodePtr() {
		return &rightNode;
	}

	void setParent(NodeType *parent) {
		this->parent = parent; 
	}

	TreeNode() {
		leftNode = 0;
		rightNode = 0;
	}

	~TreeNode() {
		delete leftNode;
		delete rightNode;
	}
};

template <typename BinaryTreeDataType>
class BinaryTree {
private:
	TreeNode<BinaryTreeDataType> *rootNode;

	void createNode(std::vector<BinaryTreeDataType> *sortedValues, size_t start, size_t end, TreeNode<BinaryTreeDataType> **nodeOut) {
		*nodeOut = new TreeNode<BinaryTreeDataType>();
		size_t mid = (start + end) / 2;
		(*nodeOut)->setParent(&sortedValues->at(mid));
		if (mid > start) {
			createNode(sortedValues, start, mid - 1, (*nodeOut)->getLeftNodePtr());
		}
		if (mid < end) {
			createNode(sortedValues, mid + 1, end, (*nodeOut)->getRightNodePtr());
		}
	}

public:
	BinaryTree(std::vector<BinaryTreeDataType> *sortedValues) {
		createNode(sortedValues, 0, sortedValues->size() - 1, &rootNode);
	}

	TreeNode<BinaryTreeDataType> *getRootNode() {
		return rootNode;
	}
	
	BinaryTree() {
		rootNode = 0;
	}

	~BinaryTree() {
		delete rootNode;
	}
};

#endif