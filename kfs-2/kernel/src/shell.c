#include "shell.h"
#include "vga.h"
#include "keyboard.h"

#define SHELL_BUFFER_SIZE 256

// We re-use a simple scancode-to-ASCII mapping (similar to keyboard.c)
static const char scancode_to_ascii[] = {
    0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 0,
    0, 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*', 0, ' '
};

/* External terminal output routines. */
extern void terminal_putchar(char c);
extern void terminal_write(const char *str);

/* A few very basic string utility functions */
static int strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const unsigned char *)s1 - *(const unsigned char *)s2;
}

static int strncmp(const char *s1, const char *s2, size_t n) {
    while(n && *s1 && (*s1 == *s2)) {
        s1++;
        s2++;
        n--;
    }
    if(n == 0)
        return 0;
    else
        return *(const unsigned char *)s1 - *(const unsigned char *)s2;
}

/*
 * shell_getchar: busy-polls the keyboard ports for a key press.
 * It returns a simple ASCII character based on the scancode.
 */
static char shell_getchar(void) {
    char c = 0;
    while (1) {
        if (inb(KEYBOARD_STATUS_PORT) & 0x01) {
            uint8_t scancode = inb(KEYBOARD_DATA_PORT);
            // Only process key-press events (ignore releases)
            if (scancode < sizeof(scancode_to_ascii) && !(scancode & 0x80)) {
                c = scancode_to_ascii[scancode];
                // Wait until key is released to avoid auto-repeat issues.
                while (inb(KEYBOARD_STATUS_PORT) & 0x01);
                break;
            }
        }
    }
    return c;
}

/*
 * shell_run: implements a simple shell that shows a prompt,
 * waits for a line of input, and then executes some built-in commands.
 */
void shell_run(void) {
    char buffer[SHELL_BUFFER_SIZE];
    size_t pos = 0;

    // Infinite loop for the shell.
    while (1) {
        // Print the prompt.
        terminal_write("shell> ");
        pos = 0;
        
        // Read a line of input.
        while (1) {
            char c = shell_getchar();
            
            // Handle backspace (assumed ASCII 0x08) if desired.
            if (c == 0x08 || c == '\b') {
                if (pos > 0) {
                    pos--;
                    terminal_putchar('\b');
                    terminal_putchar(' ');
                    terminal_putchar('\b');
                }
                continue;
            }
            
            // On RETURN/newline, end the string.
            if (c == '\n') {
                terminal_putchar('\n');
                break;
            }

            // Otherwise, echo the character and store it.
            if (pos < SHELL_BUFFER_SIZE - 1) {
                buffer[pos++] = c;
                terminal_putchar(c);
            }
        }
        buffer[pos] = '\0';  // NULL-terminate the string

        // Process the command:
        if (strcmp(buffer, "help") == 0) {
            terminal_write("Built-in commands:\n");
            terminal_write("  help      - Show this help message\n");
            terminal_write("  echo TEXT - Print TEXT\n");
            terminal_write("  exit      - Exit the shell\n");
        } else if (strncmp(buffer, "echo ", 5) == 0) {
            terminal_write(buffer + 5);
            terminal_putchar('\n');
        } else if (strcmp(buffer, "exit") == 0) {
            terminal_write("Exiting shell...\n");
            break;
        } else if (buffer[0] != '\0') {
            terminal_write("Command not found\n");
        }
    }
}
