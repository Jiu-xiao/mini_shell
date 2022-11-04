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

## 使用方法

    #include "ms.h"

    int show_fun(const char *, uint32_t len){
        ...
    }

    char get_char(){
        ...
    }

    int main(){
        ms_init(show_fun);        // 初始化时指定打印函数

        while(1){
            ms_input(get_char()); // 每次输入一个字节
            delay();              // ms_input为非阻塞，请按照一定频率调用。
        }
    }

## 添加文件/文件夹

    ms_item_t file,dir,cmd; //请确保变量存活

    ms_dir_init(&dir,"dir_name"); //初始化文件夹

    //初始化文件
    ms_file_init(&file,"file_name",NULL,file_write_fun,file_read_fun);

    //初始化命令
    ms_file_init(&cmd,"cmd_name",cmd_fun,NULL,NULL);

    ms_item_add(&file,&dir); //添加文件
    ms_item_add(&file,ms_get_root_dir()); //添加文件夹到根目录
    ms_cmd_add(&cmd); //添加命令

对于`void ms_file_init(file,name,run_fun,write_fun,read_fun)`来说，文件与命令在底层实现上是一样的，如果命令注册了读写函数可以当作文件读写，同样文件也可以被执行。

## 其他API

    void ms_enter(); //换行

    void ms_clear(); //清屏

    void ms_clear_line(); //清空光标所在行

    void ms_printf(const char* format,...) //格式化打印

    ms_item_t* ms_get_root_dir(); //获取根目录对象指针

    ms_item_t* ms_get_etc_dir(); //获取etc对象指针

    ms_item_t* ms_get_dev_dir(); //获取dev对象指针

    ms_item_t* ms_get_bin_dir(); //获取bin对象指针
