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
