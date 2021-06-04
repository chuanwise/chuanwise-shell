# 控制台：chuanwise-shell
西北大学操作系统作业，西北大学信息学院参赛项目。

请遵循 `MIT` 开源协议使用本 `Shell`。
欢迎学弟学妹们在未来写这个作业的时候参考本项目 `(๑•̀ㅂ•́)و✧`

## 特性
1. **简单的词法分析器：Splitter**<br>
除满足普通的参数解析需求，还支持添加关键字，实现 `>a-file.txt` 分割成 `>` `a-file.txt` 的功能。
关键字按最长匹配，不会将 `>>a-file.txt` 分解为 `>` `>` `a-file.txt`。

1. **指令注册机制**<br>
常见的指令处理形式并不将指令处理代码直接写在控制台的输入解析部分，而多通过一种称为指令注册的方式。简而言之，在初始化时通过为每一个指令名注册指令处理器（本程序较为简单，仅使用 `std::function<void(Command)>` 表示指令处理器），动态地为每个指令分配处理函数。

1. **支持大部分重定向写法**<br>
本 `Shell` 支持使用 `>` `>>` `1>` `1>>` `2>` `2>>` `&1` `&2` 和 `<` 自定义输入输出和错误输出`。

## 代码结构
### Command
代表一次有效的输入，作为参数传递给指令处理器。
需要注意的是，使用管道等操作的多个指令，每一个指令是一个 `Command`。

一个指令由 `指令头` 和剩余参数组成。例如 `shutdown -s -t 600` 中，`shutdown` 是指令头，`[-s, -t, 600]` 是参数。

#### 构造函数
参数类型|含义
---|---
`std::vector<std::string> tokens`|`tokens` 序列
`std::string head`|指令头

#### 成员函数
下表是主要的函数表

类型|函数名|参数列表|说明
---|---|---|---
`void`|`add_argument`|`std::string argument`|追加一个参数
`std::string`|`get_remain_arguments`|`int begin = 0`|获得从第 `begin` 开始的参数组成的字符串
`std::string`|`to_original_string`|无|获得原输入字符串

### Shell
抽象类，控制台本体。管理各种指令的处理器、解析输入等操作。

#### 成员函数
主要的函数有：
类型|函数名|参数列表|说明
---|---|---|---
`bool`|`on_command`|`std::string input`|执行输入为 `input` 时的操作（可能因使用管道被分解为多个 `Command`）
`bool`|`on_command`|`Command command`|执行输入为 `command` 时的操作
`bool`|`register_command`|`std::string head, std::function<void(Command)> executor`|注册一个对指令头 `head` 的处理器。

### LinuxShell
Shell 的一个子类，注册了一些常用简单指令，如 `ls` `cat` `echo` `pause`。

### Splitter
`Token` 解析器，可以理解为将字符串切割为一个个单词的工具。

#### 成员
类型|名称|说明
---|---|---
`std::vector<std::string>`|`keywords`|关键字列表，即必须单独识别的单词。例如重定向运算符 `>`
`std::string`|`board`|可以作为界符出现的符号。例如带有空格的参数 `"argument with spaces"` 的界符是 `"`。
`std::string`|`space`|可以作为空格出现的符号。它和 `board` 一起用于分割参数。

#### 成员函数
类型|函数名|参数列表|说明
---|---|---|---
`std::vector<std::string>`|`split`|`std::string input`|将输入 `input` 划分为 `Token`
`void`|`add_keyword`|`std::string keyword`|添加一个新的单词作为关键字

## 自带的基础指令
### ls
```bash
$ ls
chuanwise-shell.cpp  chuanwise-shell.out
$ ls ..
chuanwise-shell  cpp-lab
$ ls / 
bin   data  etc   lib    lib64   lost+found  mnt  proc      root  sbin  sys  usr
boot  dev   home  lib32  libx32  media       opt  recovery  run   srv   tmp  var
$ ls -l
总用量 252
-rw-r--r-- 1 chuanwise chuanwise  21042 5月  11 19:35 chuanwise-shell.cpp
-rwxr-xr-x 1 chuanwise chuanwise 230392 5月  11 19:20 chuanwise-shell.out
$ 
```
### cat
```bash
$ cat qwq 
cat: qwq: No such file or directory
$ echo qwq >out
$ cat out
qwq
```
### help
```bash
$ help
Can not open the help document, see it in github: https://github.com/Chuanwise/chuanwise-shell
```
偷懒了嘿嘿嘿
### echo
```bash
$ echo orz
orz
$ ls 
chuanwise-shell.cpp  chuanwise-shell.out
$ echo qwq > out
$ cat out
qwq
```
### environ
```bash
$ environ PATH
PATH = /usr/local/bin:/usr/bin:/bin:/usr/local/games:/usr/games:/sbin:/usr/sbin
```
单独输入 `$ environ` 列举所有的环境变量。其值太长就在这展示了。
### pause
```bash
$ pause
press enter to continue
```
### quit
退出 `Shell`