# 控制台：chuanwise-shell
西北大学操作系统作业

请遵循 `GNU` 开源协议使用本 `Shell`。
欢迎学弟学妹们在未来写这个作业的时候参考本项目 `(๑•̀ㅂ•́)و✧`

## 特性
1. **简单的词法分析器：Spliter**<br>
传统的编译器的词法分析是通过非确定的有限状态自动机（NFA）实现的。本项目的词法分析器参考了这个设计，但采用稍加改进的有限状态自动机实现，令其在简单、满足需求的同时，能实现最长关键字匹配。<br>
可以通过设置 `Shell.spilter` 修改指令的解析方式。

1. **指令注册机制**<br>
常见的指令处理形式并不将指令处理代码直接写在控制台的输入解析部分，而多通过一种称为指令注册的方式。简而言之，在初始化时通过为每一个指令名注册指令处理器（本 `Shell` 较为简单，仅使用 `std::function<void(Command)>` 表示指令处理器），动态地为每个指令分配处理函数。

1. **支持大部分重定向写法**<br>
本 `Shell` 支持使用 `>` `>>` `1>` `1>>` `2>` `2>>` `&1` `&2` 和 `<` 自定义输入输出和错误输出。分别对应 `std::cin` `std::cout` 和 `std::cerr`。

## 代码结构
* `Command`：一次有效的输入
* `Shell`：控制台的基类
* `LinuxShell`：默认注册了一些基础指令的 `Shell`。

## 自带的基础指令
### ls
```bash
chuanwise@chuanwise-pc:/home/chuanwise/program-language/cpp/chuanwise-shell$ ls
chuanwise-shell.cpp  chuanwise-shell.out
chuanwise@chuanwise-pc:/home/chuanwise/program-language/cpp/chuanwise-shell$ ls ..
chuanwise-shell  cpp-lab
chuanwise@chuanwise-pc:/home/chuanwise/program-language/cpp/chuanwise-shell$ ls / 
bin   data  etc   lib    lib64   lost+found  mnt  proc      root  sbin  sys  usr
boot  dev   home  lib32  libx32  media       opt  recovery  run   srv   tmp  var
chuanwise@chuanwise-pc:/home/chuanwise/program-language/cpp/chuanwise-shell$ ls -l
总用量 252
-rw-r--r-- 1 chuanwise chuanwise  21042 5月  11 19:35 chuanwise-shell.cpp
-rwxr-xr-x 1 chuanwise chuanwise 230392 5月  11 19:20 chuanwise-shell.out
chuanwise@chuanwise-pc:/home/chuanwise/program-language/cpp/chuanwise-shell$ 
```
### cd
```bash
chuanwise@chuanwise-pc:/home/chuanwise/program-language/cpp/chuanwise-shell$ cd ..
chuanwise@chuanwise-pc:/home/chuanwise/program-language/cpp$ cd ..
chuanwise@chuanwise-pc:/home/chuanwise/program-language$ cd cpp
chuanwise@chuanwise-pc:/home/chuanwise/program-language/cpp$ cd cpp-lab
```
### cat
```bash
chuanwise@chuanwise-pc:/home/chuanwise/program-language/cpp/cpp-lab$ cat qwq 
cat: qwq: No such file or directory
chuanwise@chuanwise-pc:/home/chuanwise/program-language/cpp/cpp-lab$ echo qwq >out
chuanwise@chuanwise-pc:/home/chuanwise/program-language/cpp/cpp-lab$ cat out
qwq
```
### help
```bash
chuanwise@chuanwise-pc:/home/chuanwise/program-language/cpp/chuanwise-shell$ help
Can not open the help document, see it in github: https://github.com/Chuanwise/chuanwise-shell
```
偷懒了嘿嘿嘿
### echo
```bash
chuanwise@chuanwise-pc:/home/chuanwise/program-language/cpp/chuanwise-shell$ echo orz
orz
chuanwise@chuanwise-pc:/home/chuanwise/program-language/cpp/chuanwise-shell$ ls 
chuanwise-shell.cpp  chuanwise-shell.out
chuanwise@chuanwise-pc:/home/chuanwise/program-language/cpp/chuanwise-shell$ echo qwq > out
chuanwise@chuanwise-pc:/home/chuanwise/program-language/cpp/chuanwise-shell$ cat out
qwq
```
### environ
```bash
chuanwise@chuanwise-pc:/home/chuanwise/program-language/cpp/chuanwise-shell$ environ PATH
PATH = /usr/local/bin:/usr/bin:/bin:/usr/local/games:/usr/games:/sbin:/usr/sbin
```
单独输入 `$ environ` 列举所有的环境变量。其值太长就在这展示了。
### pause
```bash
chuanwise@chuanwise-pc:/home/chuanwise/program-language/cpp/chuanwise-shell$ pause
press enter to continue
```
### quit
退出 `Shell`