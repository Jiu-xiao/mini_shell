# Mini Shell

无需操作系统与动态内存分配的嵌入式Shell

## 演示

![image](./doc/shell.gif)

## 功能

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
        ms_init(show_fun);

        while(1){
            ms_input(get_char());
            delay();
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
