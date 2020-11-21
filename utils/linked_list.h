/**************************************************************
 * Class:  CSC-415-1# FALL 2020
 * Name: Rigoberto Perez
 * Student ID: 902426361
 * GitHub UserID: Rig06
 * Project: Assignment 4 – Word-Blast
 *
 * File: linked_list.h
 *
 * Description: Declaration of prototype functions for the b_io.c file
 *
 **************************************************************/
#ifndef LINKEDLIST_LINKED_LIST_H_
#define LINKEDLIST_LINKED_LIST_H_
#include <stdio.h>
#include <stdlib.h>
#include "../b_io.h"

typedef struct Node{
	struct Node * next;
	sector_info data;
} Node;
extern Node *head;

void addNode(Node **head, sector_info val);
void deleteList(Node **head);
int returnSectorSize();
int nodeCounter();
Node* getNthNode(Node* head, int index);

void printList(Node *head);
#endif /* LINKEDLIST_LINKED_LIST_H_ */
