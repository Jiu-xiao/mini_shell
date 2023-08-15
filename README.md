# Mini Shell

无需操作系统与动态内存分配的嵌入式Shell

## 所需资源

在stm32f103上开启全部功能，使用arm-none-eabi-gcc编译器，02优化选项，sram占用1.1kb，flash占用7.3kb。

## 演示

![image](./doc/shell.gif)

## 功能

* 接近真正命令行的输入体验
* 模拟文件系统
* 命令与文件名自动补全
* 历史命令索引
* 自带基本命令

## 配置文件

参照config/ms_config_template.h编写ms_config.h并加入include路径中

```c
/* 最大命令长度 */
#define MS_MAX_CMD_LENGTH (64)

/* 命令参数上限 */
#define MS_MAX_ARG_NUM (5)

/* 历史命令数量 */
#define MS_MAX_HISTORY_NUM (8)

/* 命令行打印缓冲区长度 */
#define MS_WIRITE_BUFF_SIZE (128)

/* cat命令缓冲区 */
#define MS_CAT_BUFF_SIZE (128)

/* 自定义颜色 */
#define MS_HEAD_COLOR MS_COLOR_GREEN

/* 系统名称 */
#define MS_OS_NAME "mini-shell"

/* 用户名称 */
#define MS_USER_NAME "jiu-xiao"

/* 欢迎信息 */
#define MS_HELLO_MESSAGE "Welcome to use Mini Shell!"

/* 登陆命令 */
#define MS_INIT_COMMAND ""

/* 内置文件选择编译 */
#define MS_FILE_TTY (1)

#define MS_FILE_README (1)

/* 内置命令选择编译 */
#define MS_CMD_PWD (1)

#define MS_CMD_LS (1)

#define MS_CMD_CD (1)

#define MS_CMD_CAT (1)

#define MS_CMD_ECHO (1)

#define MS_CMD_CLEAR (1)

```

## 使用方法

```c
#include "ms.h"

int show_fun(const char *, size_t len){
    ...
}

char get_char(){
    ...
}

int main(){
    ms_init(show_fun); //初始化并指定打印函数

    ms_start(); //打印开始界面

    while(1){
        ms_input(get_char()); // 每次输入一个字节
        delay();              // ms_input为非阻塞，请按照一定频率调用。
    }
}
```

## 添加文件/文件夹

```c
ms_item_t file,dir,cmd; //请确保变量存活

char file_data[10];

ms_dir_init(&dir,"dir_name"); //初始化文件夹

//初始化文件
ms_file_init(&file,"file_name",NULL,file_data,sizeof(file_data),true);

//初始化命令
ms_file_init(&cmd,"cmd_name",cmd_fun,NULL,0,false);

ms_item_add(&file,&dir); //添加文件到文件夹
ms_item_add(&file,ms_get_root_dir()); //添加文件夹到根目录
ms_cmd_add(&cmd); //添加命令
```


对于`void ms_file_init(file,name,run_fun,data_ptr,data_size,writeable)`来说，文件与命令在底层实现上是一样的，如果命令注册了数据指针可以当作文件读写，同样文件也可以被执行。

## 用户函数

```c
typedef int (*ms_cmd_fun_t)(ms_item_t*, int argc, char* argv[]);
typedef int (*ms_write_fun_t)(struct ms_item_struct* self, const void* buf, size_t count);
typedef int (*ms_read_fun_t)(struct ms_item_struct* self, void* buf, size_t count);
```

传入的ms_item_t*为对应文件的指针，如果定义在对象或者结构体内，可以用ms_container_of得到容器指针。

例如：

```c
typedef struct{
    ms_item_t file_item;
    ...
}data_t;
```

使用ms_container_of(item_ptr,data_t,file_item)可以得到data_t的指针

## 初始化目录/文件/设备

```c
void ms_dir_init(ms_item_t* dir, const char* name);

void ms_file_init(ms_item_t* file, const char* name, ms_cmd_fun_t run_fun, void* data, size_t size, bool writeable);

void ms_dev_init(ms_item_t* file, const char* name, ms_write_fun_t write_fun, ms_read_fun_t read_fun);
```

## 其他API

```c
void ms_enter(); //换行

void ms_clear(); //清屏

void ms_clear_line(); //清空光标所在行

void ms_printf(const char* format,...); //格式化打印

//格式化打印到命令行上方（例如在使用终端的同时打印Log）
void ms_printf_insert(const char* format, ...);

ms_item_t* ms_get_root_dir(); //获取根目录对象指针

ms_item_t* ms_get_etc_dir(); //获取etc对象指针

ms_item_t* ms_get_dev_dir(); //获取dev对象指针

ms_item_t* ms_get_bin_dir(); //获取bin对象指针

ms_item_t* ms_get_home_dir(); //获取home对象指针

ms_item_t* ms_get_userhome_dir(); //获取userhome对象指针

//解析目录路径，成功返回MS_OK(0)，并写入到ans
ms_status_t ms_path_to_file(const char* path, ms_item_t** ans);

//解析文件路径，成功返回MS_OK(0)，并写入到ans
ms_status_t ms_path_to_dir(const char* raw_path, ms_item_t** ans);
```
