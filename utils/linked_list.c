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


}
/*
 *
 */
long get_file_size(Node *head){
	int fileSize = 0;
	struct Node *cur_node = head;
	while (cur_node != NULL ) {
		fileSize += cur_node->data.sector_size;
		cur_node = cur_node->next;
	}
	return fileSize;
}
/*
 * Iterates through list to
 * find the number of nodes present
 * Returns the number of nodes found
 */
int get_list_size(Node *head){

	if(head == NULL) return 0;
	int nodeCounter = 0;
	struct Node *cur_node = head;
	while (cur_node != NULL ) {
		cur_node = cur_node->next;
		nodeCounter++;
	}
	return nodeCounter;
}
/*
 * caller:
 * struct Node* head = NULL;
 * printf("\n Deleting linked list");
 * deleteList(&head);
 */
void deleteList(Node **head){
    Node *current = *head;
    Node *next;

    while (current != NULL) {
        next = current->next;

        free(current->data.buff_sector);
        current->data.buff_sector = NULL;
        free(current);
        current = NULL;

        current = next;
    }
    current = NULL;
    *head = NULL;

  //  tail = NULL;

	sector_size = 0;
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
/*
 *
 */
Node* removeLastNode(Node* head){
	if (head == NULL)
		return NULL;

	if (head->next == NULL) {
		free (head);
		head = NULL;
		return NULL;
	}

	// Find the second last node
	Node* last_node = NULL;
	Node* second_last = head;

	while (second_last->next->next != NULL)
		second_last = second_last->next;


	last_node = second_last->next;
	// Delete last node
	free (second_last->next);
	second_last->next = NULL;
	// Change next of second last
	second_last->next = NULL;

	return last_node;
}
// END
