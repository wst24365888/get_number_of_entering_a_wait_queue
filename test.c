#include <stdio.h>
#include <syscall.h>
#include <unistd.h>

#define NUMBER_OF_IO_ITERATIONS 6
#define NUMBER_OF_ITERATIONS 99999999

#define __NR_get_number_of_context_switches 450
#define __NR_get_number_of_entering_a_wait_queue 451

int main()
{
    char c;
    int i, t = 2, u = 3, v;
    unsigned int w;
    pid_t pid = getpid();

    for (i = 0; i < NUMBER_OF_IO_ITERATIONS; i++)
    {
        v = 1;
        c = getchar();
    }

    for (i = 0; i < NUMBER_OF_ITERATIONS; i++)
        v = (++t) * (u++);

    if (syscall(__NR_get_number_of_context_switches, pid, &w) != 0)
        printf("Error (1)!\n");
    else
        printf("This process encounters %u times context switches.\n", w);

    if (syscall(__NR_get_number_of_entering_a_wait_queue, pid,&w) != 0)
        printf("Error (2)!\n");
    else
        printf("This process enters a wait queue %u times.\n", w);

    for (i = 0; i < NUMBER_OF_IO_ITERATIONS; i++)
    {
        v = 1;
        printf("I love my home.\n");
    }

    if (syscall(__NR_get_number_of_entering_a_wait_queue, pid, &w) != 0)
        printf("Error (3)!\n");
    else
        printf("This process enters a wait queue %u times.\n", w);
}