#include <stdio.h>
#include <string.h>
#include "src/etc/declarations.h"
#include "src/etc/strings.c"
#include "src/etc/stdlib/G_stdio.c"
#include "src/etc/stdlib/sleep.c"

int main()
{
    //printf("%d %lf\n", is_NaN((number64) G_NaN),  (number64) G_NaN);
    //printf("%d %lf\n", is_NaN((number64)-G_NaN),  (number64)-G_NaN);
    //printf("%d %lf\n", is_NaN(1), 1.0);
    String string = init_String();
    char* msg = "this should\0 show without \0 problem";
    stringconcat_char_w_len(&string, msg, strlen(msg));
    //G_printf("%lu\n", G_printf("Hello, World 0x%lx %s 0x%p!\n", 1415135, string, "in an intersteller buuuurst, im back to save the universe", 'y'));
    G_printf("%lu\n", G_printf("Hello, World!\n", string));
    G_sleep(1000/2);
    G_printf("peace out\n");
    return 0;
}