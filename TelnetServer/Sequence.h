#ifndef __ESC_SEQ__
#define __ESC_SEQ__

#include <stdint.h>
#include <stdbool.h>

/* Codes for 1B keys*/
/* https://www.cs.cmu.edu/~pattis/15-1XX/common/handouts/ascii.html */
typedef enum {

    BACKSPACE = 8,
    TAB_KEY = 9,
    ENTER_KEY = 13,
    SPACE_KEY = 32,
    FORWARD_SLASH_KEY = 47,
    QUESTION_KEY = 63,
    PIPE_KEY = 124,
    DELETE_KEY = 127
} KEY_T;

void
esc_seq_read_cur_pos (unsigned char *msg, uint16_t msg_size, int *row, int *col);
int
esc_seq_request_cur_pos(int sockfd);
int
esc_seq_set_cur_pos(int sockfd, int row, int col);
int
esc_seq_save_cur_pos(int sockfd);
int
esc_seq_restore_cur_pos(int sockfd);
int
esc_seq_move_cur_up(int sockfd, int count);
int
esc_seq_move_cur_down(int sockfd, int count);
int
esc_seq_move_cur_left(int sockfd, int count);
int
esc_seq_move_cur_right(int sockfd, int count);
int
esc_seq_move_cur_beginning_of_line(int sockfd, int count);
int
esc_seq_move_cur_to_column(int sockfd, int col);
int
esc_seq_clear_screen(int sockfd);
int
esc_seq_erase_curr_line(int sockfd);
int
esc_seq_set_cur_at_home(int sockfd);

#endif