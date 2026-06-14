#include <stdio.h>
#include <stddef.h>
#include "etc/input.c"
#include "etc/list.c"
#include "etc/strings.c"

int main()
{
    List list = init_List(sizeof(String));

    for (char i = 0; i < 10; i++) {
        String x = init_String();
        stringconcat_charptr(&x, "Hello, World!");
        add_List(&list, &x);
    }

    for (char i = 0; i < list.length; i++) {
        printf("%d\n", (i * list.__element_size__ + list.memory) - (size_t)list.memory  + offsetof(String, content)-1);
        printf("Char: %c\n", *((char*)(i * list.__element_size__ + list.memory + offsetof(String, content))));
        //printf("%.*s\n", (list.memory + i * list.__element_size__ + offsetof(String, length)), (char*)(list.memory + i * list.__element_size__ + offsetof(String, content)));
    }

    return 0;
}