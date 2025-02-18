#include "types.h"
#include "vga.h"
#include "keyboard.h"

static uint16_t* const VGA_MEMORY = (uint16_t*)0xB8000;
static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;

static size_t terminal_row;
static size_t terminal_column;
static uint8_t terminal_color;

// ASCII Art Header
const char* HEADER[] = {
    "    AAAA    N   N  TTTTT  H   H  RRRR   OOO  DDDD   RRRR  ",
    "   A    A   NN  N    T    H   H  R   R O   O D   D  R   R ",
    "   AAAAAA   N N N    T    HHHHH  RRRR  O   O D   D  RRRR  ",
    "   A    A   N  NN    T    H   H  R  R  O   O D   D  R  R  ",
    "   A    A   N   N    T    H   H  R   R  OOO  DDDD   R   R ",
    "",
    "                     Anthrodr - KFS-1                     ",
    "",
    NULL
};


// Make terminal_putchar visible to other files
void terminal_putchar(char c);

static void terminal_initialize(void) 
{
    terminal_row = 0;
    terminal_column = 0;
    terminal_color = vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);

    // Clear screen
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = y * VGA_WIDTH + x;
            VGA_MEMORY[index] = vga_entry(' ', terminal_color);
        }
    }
}

static void terminal_putchar_at(char c, uint8_t color, size_t x, size_t y) 
{
    const size_t index = y * VGA_WIDTH + x;
    VGA_MEMORY[index] = vga_entry(c, color);
}

static size_t strlen(const char* str) {
    size_t len = 0;
    while (str[len])
        len++;
    return len;
}

void terminal_putchar(char c) 
{
    if (c == '\n') {
        terminal_column = 0;
        terminal_row++;
        if (terminal_row == VGA_HEIGHT)
            terminal_row = 0;
        return;
    }

    terminal_putchar_at(c, terminal_color, terminal_column, terminal_row);
    
    if (++terminal_column == VGA_WIDTH) {
        terminal_column = 0;
        if (++terminal_row == VGA_HEIGHT)
            terminal_row = 0;
    }
}

static void terminal_write(const char* str) 
{
    for (size_t i = 0; str[i] != '\0'; i++)
        terminal_putchar(str[i]);
}

static void terminal_writestr_centered(const char* str, size_t row) 
{
    size_t len = strlen(str);
    size_t col = (VGA_WIDTH - len) / 2;
    
    uint8_t colors[] = {
        vga_entry_color(VGA_COLOR_LIGHT_BLUE, VGA_COLOR_BLACK),
        vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK),
        vga_entry_color(VGA_COLOR_CYAN, VGA_COLOR_BLACK),
        vga_entry_color(VGA_COLOR_BLUE, VGA_COLOR_BLACK)
    };

    for (size_t i = 0; str[i]; i++) {
        size_t color_index = (i + row) % 4;
        terminal_putchar_at(str[i], colors[color_index], col + i, row);
    }
}

static void print_header(void)
{
    for (size_t i = 0; HEADER[i] != NULL; i++) {
        terminal_writestr_centered(HEADER[i], i);
    }

    terminal_row = 10;
    terminal_column = 0;
}

void kernel_main(void) 
{
    terminal_initialize();
    print_header();
    
    terminal_color = vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    
    terminal_write("\nWelcome to KFS-1!\n");
    terminal_write("42 School Kernel From Scratch - v1.0\n\n");
    terminal_write("Type something...\n");

    init_keyboard();

    while (1) {
        keyboard_handler();
    }
}