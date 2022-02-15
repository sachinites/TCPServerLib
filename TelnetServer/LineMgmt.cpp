#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include "Sequence.h"
#include "LineMgmt.h"


line_t line[2];

void
lines_init () {

    line_reinit(&line[0]);
    line_reinit(&line[1]);
}

void
line_copy(line_t *src, line_t *dst) {

    memcpy(dst, src, sizeof(line_t));
}

void
line_controlled_copy(line_t *src, uint8_t start_index, 
                                    uint8_t end_index, line_t *dst) {

    uint8_t nbytes = end_index - start_index + 1;
    assert(start_index < end_index);
    line_reinit(dst);
    memcpy(dst->lbuf, src->lbuf + start_index, nbytes);
    dst->cpos = nbytes;
    dst->lpos = nbytes - 1;
    dst->n = nbytes;
}

bool
line_is_empty(line_t *line) {

    return line->n == 0;
}

void
line_reinit(line_t *line) {

    int i;
    line->cpos = 0;
    line->lpos = 0;
    line->n = 0;
    line->saved_pos = 0;

    for (i = 0; i < MAX_LINE_LENGTH; i++) {
        line->lbuf[i] = '\0';
    }
    sem_init(&line->sem0, 0, 0);
}

void
line_replace_charat(line_t *line, char new_c, uint8_t pos) {

    assert(new_c != '\0');
    assert(line->lbuf[pos] != '\0');
    assert(pos <= line->lpos);
    line->lbuf[pos] = new_c;
}

static void 
line_insert_charat_internal(line_t *line, char c, uint8_t pos)
{
    int i;
    int n = ++line->lpos;
 
    for (i = n; i >= pos; i--)
        line->lbuf[i] = line->lbuf[i - 1];

    line->lbuf[pos] = c;
    line->n++;
}

void
line_add_character(line_t *line, char c){

    int i;
    char temp;

    assert(line->n < MAX_LINE_LENGTH);
   
   /* Ist char case */
   if (line_is_empty(line)) {
       assert(line->cpos == 0);
       assert(line->lpos == 0);
       line->lbuf[0] = c;
       line->lpos = 0;
       line->n++;
       return;
   }

   /* Append case */
    if (line->cpos == line->lpos + 1) {

        line->lbuf[line->cpos] = c;
        line->lpos++;
        line->n++;
        return;
    }

    /* Add in the middle */
    if (line->cpos >= 0 && line->cpos <= line->lpos) {

       line_insert_charat_internal(line, c, line->cpos);
    }
}

void
line_del_charat(line_t *line, uint8_t pos) {

    int i;
   if (line_is_empty(line)) return;

    /* Deleting the character from end */
    if (pos == line->lpos) {
        line->lbuf[pos] = '\0';
        line->lpos--;
        line->n--;
        return;
    }

    /* Deleting the character from middle */
    if (pos >= 0 && pos < line->lpos) {

        for (i = line->cpos + 1; i <= line->lpos; i++) {
            line->lbuf[i - 1] = line->lbuf[i];
        }
        line->lpos--;
        line->n--;
    }
}

void
line_save_cpos(line_t *line) {

    line->saved_pos = line->cpos;
}

void
line_restore_cpos(line_t *line) {

    line->cpos = line->saved_pos;
}

uint8_t
line_get_saved_pos(line_t *line) {

    return line->saved_pos;
}

void
print_line(line_t *line) {

    int i, x, y;

    printf ("Line attr : lpos = %d, cpos = %d, n = %d\n", line->lpos, line->cpos, line->n);
    for (i = 0; i <= line->lpos; i++) {
        printf("%c", line->lbuf[i]);
    }
    printf ("\n");
}

int
line_rewrite(int sockfd, line_t *line) {

    return write(sockfd, (const char *)line->lbuf, line->n);
}