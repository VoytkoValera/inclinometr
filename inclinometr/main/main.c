#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>


void app_main(void)
{
    while (true) {
        printf("Hello2 from app_main!\n");
        sleep(1);
    }
}
