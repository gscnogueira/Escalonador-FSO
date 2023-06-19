#ifndef LIST_H
#define LIST_H
#define TAM_WORD  10


typedef struct process_t {
    char name [TAM_WORD];
    struct process_t* next;
} process_t;

typedef struct process_list_t {
    unsigned int size;
    process_t* front;
    process_t* back;
} process_list_t;


void plinit(process_list_t* p_list);

void plinsert(process_list_t* plist, process_t* p);

process_t* pinit(char* p_name);

void plprint(process_list_t* plist);

int plexc(process_list_t* plist, int p_aux_id);

int plexcws(process_list_t* plist, int p_aux_id);

int pexc(process_t* p, int p_aux_id);

#endif
