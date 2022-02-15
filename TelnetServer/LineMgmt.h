#ifndef __LINE_MGMT__
#define __LINE_MGMT__

#include <stdint.h>
#include <semaphore.h>

#define MAX_LINE_LENGTH 128

typedef struct line_ {

    unsigned char lbuf[MAX_LINE_LENGTH];
    uint8_t cpos;
    uint8_t lpos;
    uint8_t saved_pos;
    uint8_t n;
    sem_t sem0;
} line_t;

void
line_copy(line_t *src, line_t *dst) ;

void
line_controlled_copy(line_t *src, uint8_t start_index, 
                                    uint8_t end_index, line_t *dst);

bool
line_is_empty(line_t *line);

void
line_reinit(line_t *line);

void
line_replace_charat(line_t *line, char new_c, uint8_t pos) ;

void
line_add_character(line_t *line, char c);

void
line_del_charat(line_t *line, uint8_t pos) ;

void
print_line(line_t *line);

void
line_save_cpos(line_t *line);

void
line_restore_cpos(line_t *line);

uint8_t
line_get_saved_pos(line_t *line) ;

int
line_rewrite(int sockfd, line_t *line);

#endif
