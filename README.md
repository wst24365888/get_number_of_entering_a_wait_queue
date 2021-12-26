# Linux Project 3

## Question 1

- kernel space

```c=
#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <linux/sched.h>
#include <linux/syscalls.h>
#include <linux/resource.h>
#include <asm/errno.h>

SYSCALL_DEFINE2(get_number_of_context_switches, pid_t, pid, unsigned int *, number_of_context_switches) {
    struct task_struct *task;
    struct rusage ru;
    unsigned int ncsw;

    task = find_task_by_vpid(pid);
    if (!task) {
        return -ESRCH;
    }

    getrusage(task, RUSAGE_SELF, &ru);

    printk("---\n");
    printk("task->nvcsw: %ld\n", task->nvcsw);
    printk("task->nivcsw: %ld\n", task->nivcsw);
    printk("task->signal->nvcsw: %ld\n", task->signal->nvcsw);
    printk("task->signal->nivcsw: %ld\n", task->signal->nivcsw);
    printk("task->last_switch_count: %ld\n", task->last_switch_count);
    printk("ru.ru_nvcsw: %ld\n", ru.ru_nvcsw);
    printk("ru.ru_nivcsw: %ld\n", ru.ru_nivcsw);

    ncsw = ru.ru_nvcsw + ru.ru_nivcsw;
    if(copy_to_user(number_of_context_switches, &ncsw, sizeof(unsigned int))) {
        return -EFAULT;
    }

    return 0;
}
```

- user space

```c=
#include <stdio.h>
#include <syscall.h>
#include <unistd.h>

#define NUMBER_OF_ITERATIONS 99999999

#define __NR_get_number_of_context_switches 450

int main()
{
 int i, t = 2, u = 3, v;
 pid_t pid = getpid();
 int nvcw;

 for (i = 0; i < NUMBER_OF_ITERATIONS; i++)
  v = (++t) * (u++);

 if (syscall(__NR_get_number_of_context_switches, pid, &nvcw))
  printf("Error!\n");
 else
  printf("This process encounters %u times context switches.\n", nvcw);

 // getchar(); // cat /proc/PID/sched to validate the result

 return 0;
```

![ ](https://i.imgur.com/eq9b8MV.png)

### 在include/linux/sched.h裡的task_struct

- context switch有兩個情況 分別被兩個變數所記著
  - nvcsw
  - nivcsw

## Question 2

- kernel space

```c=
#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <linux/sched.h>
#include <linux/syscalls.h>
#include <linux/resource.h>
#include <asm/errno.h>

SYSCALL_DEFINE2(get_number_of_entering_a_wait_queue, pid_t, pid, unsigned int *, number_of_entering_a_wait_queue)
{
    struct task_struct *task;
    unsigned int newq;

    task = find_task_by_vpid(pid);
    if (!task)
    {
        return -ESRCH;
    }

    printk("---\n");
    printk("task->enter_queue_count: %d\n", task->enter_queue_count);
    newq = task->enter_queue_count;

    if (copy_to_user(number_of_entering_a_wait_queue, &newq, sizeof(unsigned int)))
    {
        return -EFAULT;
    }

    return 0;
}
```

- user space

```c=
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
        c = getchar();
    }

    for (i = 0; i < NUMBER_OF_ITERATIONS; i++)
        v = (++t) * (u++);

    for (i = 0; i < NUMBER_OF_IO_ITERATIONS; i++)
    {
        v = 1;
        printf("I love my home.\n");
    }

    if (syscall(__NR_get_number_of_context_switches, pid, &w) != 0)
        printf("Error (3)!\n");
    else
        printf("This process encounters %u times context switches.\n", w);

    if (syscall(__NR_get_number_of_entering_a_wait_queue, pid,&w) != 0)
        printf("Error (4)!\n");
    else
        printf("This process enters a wait queue %u times.\n", w);
}
```

### sched.h task_struct variable

![ ](https://i.imgur.com/OOfjHXq.png)

### 在ttwu_do_wake_up裡加上遞增條件

![ ](https://i.imgur.com/382hpWQ.png)

#### trace路徑

>[default_wake_function](https://elixir.free-electrons.com/linux/v3.9/source/kernel/sched/core.c#L3109)-> [try_to_wake_up](https://elixir.free-electrons.com/linux/v3.9/source/kernel/sched/core.c#L1439)->[ttwu_queue](https://elixir.bootlin.com/linux/v5.14.9/source/kernel/sched/core.c#L3577)-> [ttwu_do_active](https://elixir.bootlin.com/linux/v5.14.9/source/kernel/sched/core.c#L3370)-> [ttwu_do_wake_up](https://elixir.bootlin.com/linux/v5.14.9/source/kernel/sched/core.c#L3334)

- 在執行999999 entering wait queue的時候他會因為priority order變小 所以被context switch掉
- 在執行I/O的時候 只要輸入有換行 process就會進行wake up 到TASK_RUNNING狀態然後再到current 發現到仍然要等待使用者輸入 所以又會被丟回去wait_queue進行等待(只要輸入多一行 enter queue就會多一次
![](<https://i.imgur.com/El3QwBd.png> =600x)

1. 只有default的6次getchar()+999999次迴圈，沒有額外的getchar()輸入
    - 輸入1行getchar()以後會有1次context_switch，999999次迴圈內又多context_switch了1次
    - 中間沒有其他動作，兩次輸出沒有變化
![ ](https://i.imgur.com/2lKAhGD.png)

2. 除了default的6次getchar()+999999次迴圈，還多用了6次getchar()，輸入1行
    - 輸入1行getchar()以後會有1次context_switch，999999次迴圈內又多context_switch了2次
    - 中間多輸入1行getchar()，enter_wait_queue多了1次，context_switch也多了1次
![ ](https://i.imgur.com/A77jBJ5.png)
3. 除了default的6次getchar()+999999次迴圈，還多用了6次getchar()，輸入2行
    - 輸入1行getchar()以後會有1次context_switch，999999次迴圈內又多context_switch了1次
    - 中間多輸入2行getchar()，enter_wait_queue多了2次，context_switch也多了2次
![ ](https://i.imgur.com/bxB3lPC.png)
4. 除了default的6次getchar()+999999次迴圈，還多用了6次getchar()，輸入3行
    - 輸入1行getchar()以後會有1次context_switch，999999次迴圈內又多context_switch了1次
    - 中間多輸入3行getchar()，enter_wait_queue多了3次，context_switch也多了3次
![ ](https://i.imgur.com/1RUlHTY.png)
5. 除了default的6次getchar()+999999次迴圈，還多用了6次getchar()，輸入1行+第二個999999次迴圈
    - 輸入1行getchar()以後會有1次context_switch，999999次迴圈內沒有context_switch
    - 中間多輸入1行getchar()，enter_wait_queue多了1次，context_switch也多了1次，又第二個999999次迴圈內多了2次context_switch
![ ](https://i.imgur.com/mcQTIuG.png)
