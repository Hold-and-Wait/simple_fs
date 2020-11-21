/**************************************************************
 * Class:  CSC-415-1# FALL 2020
 * Name: Rigoberto Perez
 * Student ID: 902426361
 * GitHub UserID: Rig06
 * Project: Assignment 4 – Word-Blast
 *
 * File: linked_list.c
 *
 * Description: the code is template for a linkedlist data structure
 *
 **************************************************************/
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include "linked_list.h"
#include "../b_io.h"
struct Node *tail = NULL;
int sector_size = 0;
int counter = 0;
/*
 * Appends node to the end of the list
 * If the list does not exist, one is created
 */
void addNode(Node **head, sector_info new_data){

	//printf("HEAD: %s", new_data.buff_sector);

	Node *new_node = NULL;
	new_node = malloc(sizeof(struct Node));
	new_node->data = new_data; // Link data part
	if(*head == NULL){
		new_node->next = NULL;
		*head = new_node;// Link address part
		tail = new_node;
		tail->next = NULL;          // Make newNode as first node
		sector_size = tail->data.sector_size;
		return;

	}
	new_node->next = NULL;
	tail->next = new_node; // Link data part
	tail =  tail->next;
	sector_size = tail->data.sector_size;
	counter++;

}
/*
 *
 */
int nodeCounter(){
	return counter;
}
/*
 *
 */
int returnSectorSize(){
	return sector_size;
}
/*
 *
 */

// caller:
// struct Node* head = NULL;
// printf("\n Deleting linked list");
// deleteList(&head);
void deleteList(Node **head){
	/* deref head_ref to get the real head */
	struct Node* current = *head;
	struct Node* next;

	while (current != NULL) {
		next = current->next;
		free(current);
		current = next;
	}
	/* deref head_ref to affect the real head back
	      in the caller. */
	*head = NULL;
}
/*
 *
 * printing all nodes
 *
 */
void printList(Node *head){
	struct Node *cur_node = head;

	while (cur_node != NULL ) {
		printf("Bytes: %d,	Head: %s \n\n", cur_node->data.sector_size, cur_node->data.buff_sector);

		cur_node = cur_node->next;

	}
}





/* Takes head pointer of
 * the linked list and index
 * as arguments and return
 * data at index
 */
Node* getNthNode(Node* head, int index){

	if(index < 0 ){

		return NULL;
	}

	Node* current = head;
	// the index of the
	// node we're currently
	// looking at
	int count = 0;
	while (current != NULL) {
		if (count == index)break;
		count++;
		current = current->next;
	}

	return current;


}









