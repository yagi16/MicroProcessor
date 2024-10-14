/* Do not remove the following line. Do not remove interrupt_handler(). */
#include "crt0.c"
#include "ChrFont0.h"

void show_ball(int pos);
void show_cats();
void show_score();
void play();
int  btn_check_0();
int  btn_check_1();
int  btn_check_3();
void led_set(int data);
void led_blink();
void lcd_init();
void lcd_putc(int y, int x, int c);
void lcd_puts(int y, int x, char *str);
void lcd_sync_vbuf();
void lcd_clear_vbuf();
int kypd_scan();
void move_cursor(int direction);
int my_rand();
void my_itoa();

#define INIT    0
#define OPENING 1
#define PLAY    2
#define ENDING  3

#define UP_KEY         0x8
#define RIGHT_KEY      0xe
#define DOWN_KEY       0xf
#define LEFT_KEY       0x0
#define INTERACT_KEY   0x9

#define CATS_NUM 10
#define GAME_TIME 200 // rimit over 20s

int state = INIT;
int cat_state[CATS_NUM] = {0};
int cat_timer[CATS_NUM] = {0};
int cat_position[CATS_NUM][2] = {0};
unsigned int randNum; // seed
int toy_position = 0; // init_pos
int score = 0;
int game_timer;

/* interrupt_handler() is called every 100msec */
void interrupt_handler() {
    static int spawn_timer;
    static int cnt = 0;
    volatile int *seg7_ptr = (int *)0xff18;
    cnt++;
    if (cnt % 10 == 0)
	*seg7_ptr = cnt / 10;

    if (state == INIT) {
    } else if (state == OPENING) {
        spawn_timer = 0;
        game_timer = GAME_TIME;
    } else if (state == PLAY) {
        if (spawn_timer <= 0) {
            int idx = my_rand();
            cat_state[idx] = 1;
            cat_timer[idx] = 10;
            spawn_timer = 10;
        } else {
            spawn_timer--;
        }

        for (int i = 0; i < CATS_NUM; i++) {
            if (cat_state[i] == 1) {
                if (cat_timer[i] <= 0) {
                    cat_state[i] = 0;
                } else {
                    cat_timer[i]--;
                }
            }
        }

        if (game_timer > 0) {
            game_timer--;
        } else {
            state = ENDING;
        }

        lcd_clear_vbuf();
        show_cats();
        int x = cat_position[toy_position][0];
        int y = cat_position[toy_position][1];
        lcd_putc(y - 1, x, 'x');
        show_score();
        lcd_sync_vbuf();

    } else if (state == ENDING) {
        lcd_clear_vbuf();
        char score_str[16];
        char prefix[] = "score:";
        char border[] = "_________";
        lcd_puts(2, 0, border);
        my_itoa(score, score_str + 6, 10);
        for (int i = 0; i < 6; i++) score_str[i] = prefix[i];
        lcd_puts(3, 0, score_str);
        lcd_puts(4, 0, border);
        lcd_sync_vbuf();
    }
}

void main() {
    while (1) {
        if (state == INIT) {
            int local_cat_position[CATS_NUM][2] = {
                {1, 3}, {3, 3}, {5, 3}, {7, 3}, {9, 3},
                {1, 6}, {3, 6}, {5, 6}, {7, 6}, {9, 6}
            };
            for (int i = 0; i < CATS_NUM; i++) {
                cat_position[i][0] = local_cat_position[i][0];
                cat_position[i][1] = local_cat_position[i][1];
            }
            randNum = 1100; // seed

            lcd_init();
            game_timer = GAME_TIME;
            state = OPENING;
        } else if (state == OPENING) {
            if (-1 != kypd_scan()) {
                state = PLAY;
            } 
        } else if (state == PLAY) {
            play();
            state = ENDING;
        } else if (state == ENDING) {
            state = OPENING;
        }
    }
}

void play() {
    int prev_key_state = -1;
    int repeat_delay = 900;
    int repeat_rate = 500;
    int repeat_counter = 0;

    while (1) {
        int direction = kypd_scan();
        if (direction != -1) {
            if (prev_key_state != direction) { // new input
                if (direction == INTERACT_KEY) { // key 9 is pressed
                    if (cat_state[toy_position] == 1) {
                        cat_state[toy_position] = 0;
                        led_blink();
                        score++;
                    }
                } else { // other key is pressed
                    move_cursor(direction);
                }
                repeat_counter = repeat_delay;
            } else { // while pushing the same key
                if (repeat_counter <= 0) {
                    move_cursor(direction);
                    repeat_counter = repeat_rate;
                } else {
                    repeat_counter--;
                }
            }
        } else {
            repeat_counter = 0;
        }
        prev_key_state = direction;
        for (int i = 0; i < 1000; i++);
    }
}

/*
 * Display function
 */
void show_cats() {
    for (int i = 0; i < CATS_NUM; i++) {
        int x = cat_position[i][0];
        int y = cat_position[i][1];
        if (cat_state[i] == 1) {
            lcd_putc(y, x, 'C');
        } else if (cat_state[i] == 0) {
            lcd_putc(y, x, '_');
        }
    }
}

void show_score() {
    char score_str[16];
    char prefix[] = "score:";
    my_itoa(score, score_str + 6, 10);
    for (int i = 0; i < 6; i++) score_str[i] = prefix[i];
    lcd_puts(0, 0, score_str);
}

void show_opening() {
    lcd_clear_vbuf();
    
    lcd_puts(1, 0, "Whack-a-mole");
    lcd_puts(4, 5, "^");
    lcd_puts(5, 5, "89");
    lcd_puts(6, 3, "<0FE>");
    
    lcd_sync_vbuf();
}

/*
 * Switch functions
 */
int btn_check_0() {
    volatile int *sw_ptr = (int *)0xff04;
    return (*sw_ptr & 0x10) ? 1 : 0;
}

int btn_check_1() {
    volatile int *sw_ptr = (int *)0xff04;
    return (*sw_ptr & 0x20) ? 1 : 0;
}

int btn_check_2() {
    volatile int *sw_ptr = (int *)0xff04;
    return (*sw_ptr & 0x40) ? 1 : 0;
}

int btn_check_3() {
    volatile int *sw_ptr = (int *)0xff04;
    return (*sw_ptr & 0x80) ? 1 : 0;
}

/*
 * LED functions
 */
void led_set(int data) {
    volatile int *led_ptr = (int *)0xff08;
    *led_ptr = data;
}

void led_blink() {
    led_set(0xf); // Turn on
    for (int i = 0; i < 300000; i++); // Wait
    led_set(0x0); // Turn off
    for (int i = 0; i < 300000; i++); // Wait
    led_set(0xf); // Turn on
    for (int i = 0; i < 300000; i++); // Wait
    led_set(0x0); // Turn off
}

/*
 * LCD functions
 */
unsigned char lcd_vbuf[64][96];

void lcd_wait(int n) {
    for (int i = 0; i < n; i++);
}

void lcd_cmd(unsigned char cmd) {
    volatile int *lcd_ptr = (int *)0xff0c;
    *lcd_ptr = cmd;
    lcd_wait(1000);
}

void lcd_data(unsigned char data) {
    volatile int *lcd_ptr = (int *)0xff0c;
    *lcd_ptr = 0x100 | data;
    lcd_wait(200);
}

void lcd_pwr_on() {
    volatile int *lcd_ptr = (int *)0xff0c;
    *lcd_ptr = 0x200;
    lcd_wait(700000);
}

void lcd_init() {
    lcd_pwr_on();
    lcd_cmd(0xa0);
    lcd_cmd(0x20);
    lcd_cmd(0x15);
    lcd_cmd(0);
    lcd_cmd(95);
    lcd_cmd(0x75);
    lcd_cmd(0);
    lcd_cmd(63);
    lcd_cmd(0xaf);
}

void lcd_set_vbuf_pixel(int row, int col, int r, int g, int b) {
    r >>= 5;
    g >>= 5;
    b >>= 6;
    lcd_vbuf[row][col] = ((r << 5) | (g << 2) | (b << 0)) & 0xff;
}

void lcd_clear_vbuf() {
    for (int row = 0; row < 64; row++)
        for (int col = 0; col < 96; col++)
            lcd_vbuf[row][col] = 0;
}

void lcd_sync_vbuf() {
    for (int row = 0; row < 64; row++)
        for (int col = 0; col < 96; col++)
            lcd_data(lcd_vbuf[row][col]);
}

void lcd_putc(int y, int x, int c) {
    for (int v = 0; v < 8; v++)
        for (int h = 0; h < 8; h++)
            if ((font8x8[(c - 0x20) * 8 + h] >> v) & 0x01)
                lcd_set_vbuf_pixel(y * 8 + v, x * 8 + h, 0, 255, 180);
}

void lcd_puts(int y, int x, char *str) {
    for (int i = 0; i < 12 && str[i] != '\0'; i++) {
        if (str[i] < 0x20 || str[i] > 0x7f)
            break;
        else
            lcd_putc(y, x + i, str[i]);
    }
}

/*
 * Keypad function
 */
int kypd_scan() {
    volatile int *iob_ptr = (int *)0xff14;
    *iob_ptr = 0x07; // 0111
    for (int i = 0; i < 1; i++);
    if ((*iob_ptr & 0x80) == 0) return 0x1;
    if ((*iob_ptr & 0x40) == 0) return 0x4;
    if ((*iob_ptr & 0x20) == 0) return 0x7;
    if ((*iob_ptr & 0x10) == 0) return 0x0;
    *iob_ptr = 0x0b; // 1011
    for (int i = 0; i < 1; i++);
    if ((*iob_ptr & 0x80) == 0) return 0x2;
    if ((*iob_ptr & 0x40) == 0) return 0x5;
    if ((*iob_ptr & 0x20) == 0) return 0x8;
    if ((*iob_ptr & 0x10) == 0) return 0xf;
    *iob_ptr = 0x0d; // 1101
    for (int i = 0; i < 1; i++);
    if ((*iob_ptr & 0x80) == 0) return 0x3;
    if ((*iob_ptr & 0x40) == 0) return 0x6;
    if ((*iob_ptr & 0x20) == 0) return 0x9;
    if ((*iob_ptr & 0x10) == 0) return 0xe;
    *iob_ptr = 0x0e; // 1110
    for (int i = 0; i < 1; i++);
    if ((*iob_ptr & 0x80) == 0) return 0xa;
    if ((*iob_ptr & 0x40) == 0) return 0xb;
    if ((*iob_ptr & 0x20) == 0) return 0xc;
    if ((*iob_ptr & 0x10) == 0) return 0xd;
    return -1;
}

void move_cursor(int direction) {
    if (direction == UP_KEY) { // up (8)
        if (toy_position >= 5) toy_position -= 5;
    } else if (direction == RIGHT_KEY) { // right (E)
        if (toy_position % 5 < 4) toy_position += 1;
    } else if (direction == DOWN_KEY) { // down (F)
        if (toy_position < 5) toy_position += 5;
    } else if (direction == LEFT_KEY) { // left (0)
        if (toy_position % 5 > 0) toy_position -= 1;
    }
}

/*
 * DIY function
 */
int my_rand(void) {
    randNum = (randNum * 48271) % 2147483647; // (48,271 * Xn) mod (2^31 - 1)
    return (unsigned int)(randNum % 10);
}

void my_itoa(int value, char *str, int base) {
    char *ptr = str, *ptr1 = str, tmp_char;
    int tmp_value;

    if (value < 0 && base == 10) {
        *ptr++ = '-';
        value *= -1;
    }

    tmp_value = value;
    do {
        int remainder = tmp_value % base;
        *ptr++ = (remainder < 10) ? (remainder + '0') : (remainder - 10 + 'a');
    } while (tmp_value /= base);

    *ptr = '\0';

    if (*str == '-') {
        ptr1++;
    }

    while (ptr1 < --ptr) {
        tmp_char = *ptr;
        *ptr = *ptr1;
        *ptr1 = tmp_char;
        ptr1++;
    }
}