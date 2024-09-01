#include <stdbool.h>
#include <stddef.h>
#include <threads.h>
#include <stdlib.h>

void initQueue(void);
void destroyQueue(void);
void enqueue(void *item);
void *dequeue(void);
bool tryDequeue(void **item);
size_t size(void);
size_t waiting(void);
size_t visited(void);

typedef struct Node
{
    void *data;
    struct Node *next;
} Node;

typedef struct Queue
{
    Node *head;
    Node *tail;
    size_t num_of_items;
    size_t wait_counter;
    size_t visit_counter;
    size_t serving_ticket;
    size_t next_ticket;
    mtx_t mutex;
    cnd_t not_empty;
} Queue;

static Queue *queue = NULL;

void initQueue(void)
{
    queue = (Queue *)malloc(sizeof(Queue));
    if (queue)
    {
        queue->head = queue->tail = NULL;
        queue->num_of_items = 0;
        queue->wait_counter = 0;
        queue->visit_counter = 0;
        queue->serving_ticket = 0;
        queue->next_ticket = 0;
        mtx_init(&queue->mutex, mtx_plain);
        cnd_init(&queue->not_empty);
    }
}

void destroyQueue(void)
{
    if (queue)
    {
        mtx_lock(&queue->mutex);
        while (queue->head)
        {
            Node *tmp = queue->head;
            queue->head = queue->head->next;
            free(tmp);
        }
        mtx_unlock(&queue->mutex);
        mtx_destroy(&queue->mutex);
        cnd_destroy(&queue->not_empty);
        free(queue);
        queue = NULL;
    }
}

void enqueue(void *item)
{
    if (queue)
    {
        mtx_lock(&queue->mutex);

        Node *new_node = malloc(sizeof(Node));
        new_node->data = item;
        new_node->next = NULL;

        if (queue->tail == NULL)
        {
            queue->head = queue->tail = new_node;
        }
        else
        {
            queue->tail->next = new_node;
            queue->tail = new_node;
        }

        queue->num_of_items++;

        cnd_signal(&queue->not_empty);

        mtx_unlock(&queue->mutex);
    }
}

void *dequeue(void)
{
    void *item = NULL;
    if (queue)
    {
        mtx_lock(&queue->mutex);

        size_t my_ticket = queue->next_ticket++;

        while (queue->num_of_items == 0 || my_ticket != queue->serving_ticket)
        {
            queue->wait_counter++;
            cnd_wait(&queue->not_empty, &queue->mutex);
            queue->wait_counter--;
        }

        Node *node = queue->head;
        item = node->data;
        queue->head = queue->head->next;
        if (queue->head == NULL)
        {
            queue->tail = NULL;
        }
        free(node);

        queue->num_of_items--;
        queue->serving_ticket++;
        queue->visit_counter++;
        cnd_broadcast(&queue->not_empty);

        mtx_unlock(&queue->mutex);
    }
    return item;
}

bool tryDequeue(void **item)
{
    bool flag = false;
    if (queue)
    {
        mtx_lock(&queue->mutex);

        size_t my_ticket = queue->next_ticket++;

        if (queue->num_of_items > 0 && my_ticket == queue->serving_ticket)
        {
            Node *node = queue->head;
            *item = node->data;
            queue->head = queue->head->next;
            if (queue->head == NULL)
            {
                queue->tail = NULL;
            }
            free(node);
            queue->num_of_items--;
            queue->serving_ticket++;
            queue->visit_counter++;
            flag = true;
            cnd_broadcast(&queue->not_empty);
        }
        else
        {
            *item = NULL;
            queue->next_ticket--;
        }

        mtx_unlock(&queue->mutex);
    }
    else
    {
        *item = NULL;
    }
    return flag;
}

size_t size(void)
{
    return queue ? queue->num_of_items : 0;
}

size_t waiting(void)
{
    return queue ? queue->wait_counter : 0;
}

size_t visited(void)
{
    return queue ? queue->visit_counter : 0;
}