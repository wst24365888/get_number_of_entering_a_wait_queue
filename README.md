# Linux Project 3

## Question 1

## Question 2

- sched.h task_struct variable
![OOfjHXq](https://i.imgur.com/OOfjHXq.png)
- 在ttwu_do_wake_up裡加上遞增條件
![382hpWQ](https://i.imgur.com/382hpWQ.png)
- trace路徑
- [default_wake_function](https://elixir.free-electrons.com/linux/v3.9/source/kernel/sched/core.c#L3109)-> [try_to_wake_up](https://elixir.free-electrons.com/linux/v3.9/source/kernel/sched/core.c#L1439)->[ttwu_queue](https://elixir.bootlin.com/linux/v5.14.9/source/kernel/sched/core.c#L3577)-> [ttwu_do_active](https://elixir.bootlin.com/linux/v5.14.9/source/kernel/sched/core.c#L3370)-> [ttwu_do_wake_up](https://elixir.bootlin.com/linux/v5.14.9/source/kernel/sched/core.c#L3334)
- 在執行999999 entering wait queue的時候他會因為priority order變小 所以被context switch掉
- 在執行I/O的時候 只要輸入有換行 process就會進行wake up 到TASK_RUNNING狀態然後再到current 發現到仍然要等待使用者輸入 所以又會被丟回去wait_queue進行等待(只要輸入多一行 enter queue就會多一次
![El3QwBd](<https://i.imgur.com/El3QwBd.png> =600x)

- 只有default的6次getchar()+999999次迴圈，沒有額外的getchar()輸入
  - 輸入1行getchar()以後會有1次context_switch，999999次迴圈內又多context_switch了1次
  - 中間沒有其他動作，兩次輸出沒有變化
![2lKAhGD](https://i.imgur.com/2lKAhGD.png)

- 除了default的6次getchar()+999999次迴圈，還多用了6次getchar()，輸入1行
  - 輸入1行getchar()以後會有1次context_switch，999999次迴圈內又多context_switch了2次
  - 中間多輸入1行getchar()，enter_wait_queue多了1次，context_switch也多了1次
![A77jBJ5](https://i.imgur.com/A77jBJ5.png)
- 除了default的6次getchar()+999999次迴圈，還多用了6次getchar()，輸入2行
  - 輸入1行getchar()以後會有1次context_switch，999999次迴圈內又多context_switch了1次
  - 中間多輸入2行getchar()，enter_wait_queue多了2次，context_switch也多了2次
![bxB3lPC](https://i.imgur.com/bxB3lPC.png)
- 除了default的6次getchar()+999999次迴圈，還多用了6次getchar()，輸入3行
  - 輸入1行getchar()以後會有1次context_switch，999999次迴圈內又多context_switch了1次
  - 中間多輸入3行getchar()，enter_wait_queue多了3次，context_switch也多了3次
![1RUlHTY](https://i.imgur.com/1RUlHTY.png)
- 除了default的6次getchar()+999999次迴圈，還多用了6次getchar()，輸入1行+第二個999999次迴圈
  - 輸入1行getchar()以後會有1次context_switch，999999次迴圈內沒有context_switch
  - 中間多輸入1行getchar()，enter_wait_queue多了1次，context_switch也多了1次，又第二個999999次迴圈內多了2次context_switch
![mcQTIuG](https://i.imgur.com/mcQTIuG.png)
