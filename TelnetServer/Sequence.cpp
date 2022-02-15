#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include "Sequence.h"

int
esc_seq_request_cur_pos(int sockfd) {

    /* ^[ [ 6 n */
    return write(sockfd, "\033[6n", 4);
}

void
esc_seq_read_cur_pos (unsigned char *msg, uint16_t msg_size, int *row, int *col) {

    /* msg contain :  ^[ [ # ; # R     where # are rows and col */
    assert(msg[msg_size -1] == 'R');
    sscanf((const char *)(msg + 2), "%d;%dR", row, col);
}

int
esc_seq_save_cur_pos(int sockfd) {

    return write(sockfd, "^[[s", 3);
}

int
esc_seq_restore_cur_pos(int sockfd) {

    return write(sockfd, "^[[u", 3);
}

int
esc_seq_set_cur_pos(int sockfd, int row, int col) {
    /*
    ^[ [ {line} ; {column} H
    ^[ [ {line} ; {column} f
    */
    int rc;
    char buff[16];

    rc = sprintf(buff, "\033[%d;%dH", row, col);
    return write(sockfd, buff, rc);
}

int
esc_seq_move_cur_up(int sockfd, int count) {

    /* ^[ [ # A */
    int rc;
    char buff[16];

    rc = sprintf(buff, "\033[%dA", count);
    return write(sockfd, buff, rc);
}

int
esc_seq_move_cur_down(int sockfd, int count) {

    /* ^[ [ # B */
    int rc;
    char buff[16];

    rc = sprintf(buff, "\033[%dB", count);
    return write(sockfd, buff, rc);
}

int
esc_seq_move_cur_right(int sockfd, int count) {

    /* ^[ [ # C */
    int rc;
    char buff[16];

    rc = sprintf(buff, "\033[%dC", count);
    return write(sockfd, buff, rc);
}

int
esc_seq_move_cur_left(int sockfd, int count) {

    /* ^[ [ # D */
    int rc;
    char buff[16];

    rc = sprintf(buff, "\033[%dD", count);
    return write(sockfd, buff, rc);
}

/*moves cursor to beginning of next line, # lines down */
int
esc_seq_move_cur_beginning_of_line(int sockfd, int count) {

    /* ^[ [ # E */
    int rc;
    char buff[16];

    rc = sprintf(buff, "\033[%dE", count);
    return write(sockfd, buff, rc);
}

/* To move the cursor to column 'col', pass Value 'col + 1'*/
int
esc_seq_move_cur_to_column(int sockfd, int col) {

    /* ^[ [ # G */
    int rc;
    char buff[16];

    rc = sprintf(buff, "\033[%dG", col);
    return write(sockfd, buff, rc);
}

int
esc_seq_clear_screen(int sockfd) {

    /* ^[ [ 2 J */
    return write(sockfd, "\033[2J", 4);
}

int
esc_seq_erase_curr_line(int sockfd) {

 /* ^[ [ 2 K */
    return write(sockfd, "\033[2K", 4);
}

int
esc_seq_set_cur_at_home(int sockfd) {

     return write(sockfd, "\r", 1);
}