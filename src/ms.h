#ifndef __MS_H__
#define __MS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ms_config.h"
#include "om_list.h"

#define MS_CSI(code) "\033[" #code "m" /**< ANSI CSI指令 */

/**
 * 终端字体颜色代码
 */
#define MS_COLOR_BLACK MS_CSI(30)     /**< 黑色 */
#define MS_COLOR_RED MS_CSI(31)       /**< 红色 */
#define MS_COLOR_GREEN MS_CSI(32)     /**< 绿色 */
#define MS_COLOR_YELLOW MS_CSI(33)    /**< 黄色 */
#define MS_COLOR_BLUE MS_CSI(34)      /**< 蓝色 */
#define MS_COLOR_FUCHSIN MS_CSI(35)   /**< 品红 */
#define MS_COLOR_CYAN MS_CSI(36)      /**< 青色 */
#define MS_COLOR_WHITE MS_CSI(37)     /**< 白色 */
#define MS_COLOR_BLACK_L MS_CSI(90)   /**< 亮黑 */
#define MS_COLOR_RED_L MS_CSI(91)     /**< 亮红 */
#define MS_COLOR_GREEN_L MS_CSI(92)   /**< 亮绿 */
#define MS_COLOR_YELLOW_L MS_CSI(93)  /**< 亮黄 */
#define MS_COLOR_BLUE_L MS_CSI(94)    /**< 亮蓝 */
#define MS_COLOR_FUCHSIN_L MS_CSI(95) /**< 亮品红 */
#define MS_COLOR_CYAN_L MS_CSI(96)    /**< 亮青 */
#define MS_COLOR_WHITE_L MS_CSI(97)   /**< 亮白 */
#define MS_COLOR_DEFAULT MS_CSI(39)   /**< 默认 */

typedef enum {
  MS_MODE_FILE,
  MS_MODE_DIR,
  MS_MODE_DEV,
} ms_mode_t;

typedef enum {
  MS_OK = 0,
  MS_ERROR,
  MS_ERROR_NULL,
  MS_ERROR_INIT,
  MS_ERROR_BUSY
} ms_status_t;

typedef struct ms_item_struct {
  const char* name;
  struct ms_item_struct* father;
  union {
    struct {
      om_list_head_t list;
    } as_dir;

    struct {
      void* data;
      size_t size;
      bool writeable;
      int (*run)(struct ms_item_struct* self, int argc, char* argv[]);
    } as_file;

    struct {
      int (*read)(struct ms_item_struct* self, void* buff, size_t size);
      int (*write)(struct ms_item_struct* self, const void* buff, size_t size);
    } as_device;
  } data;
  om_list_head_t self;

  ms_mode_t mode;
} ms_item_t;

typedef int (*ms_cmd_fun_t)(ms_item_t*, int argc, char* argv[]);
typedef int (*ms_write_fun_t)(struct ms_item_struct* self, const void* buf,
                              size_t count);
typedef int (*ms_read_fun_t)(struct ms_item_struct* self, void* buf,
                             size_t count);

typedef struct {
  int (*write)(const char*, size_t);
  struct {
    char write_buff[MS_WIRITE_BUFF_SIZE];
    char read_buff[MS_MAX_CMD_LENGTH];
    char prase_buff[MS_MAX_CMD_LENGTH];
    char path_prase_buff[MS_MAX_CMD_LENGTH];
    char* arg_map[MS_MAX_ARG_NUM];
    char history_buff[MS_MAX_HISTORY_NUM][MS_MAX_CMD_LENGTH];
#if MS_FILE_README
    char readme_buff[65];
#endif
#if MS_CMD_CAT
    char cat_buff[MS_CAT_BUFF_SIZE];
#endif
  } buff;

  struct {
    int16_t index;
    uint16_t last;
    uint16_t num;
  } history;

  struct {
    ms_item_t root_dir;
    ms_item_t bin_dir;
    ms_item_t dev_dir;
    ms_item_t etc_dir;
    ms_item_t home_dir;
    ms_item_t user_home_dir;
#if MS_FILE_TTY
    ms_item_t tty_dev;
#endif
#if MS_FILE_README
    ms_item_t readme_file;
#endif
#if MS_CMD_PWD
    ms_item_t pwd_cmd;
#endif
#if MS_CMD_LS
    ms_item_t ls_cmd;
#endif
#if MS_CMD_CD
    ms_item_t cd_cmd;
#endif
#if MS_CMD_CAT
    ms_item_t cat_cmd;
#endif
#if MS_CMD_ECHO
    ms_item_t echo_cmd;
#endif
#if MS_CMD_CLEAR
    ms_item_t clear_cmd;
#endif
  } sys_file;

  struct {
    size_t index;
    size_t offset;
    uint8_t ansi;
    ms_item_t* cur_dir;
    ms_item_t* tab_tmp;
    uint16_t tab_same_len;
  } ctrl;
} ms_t;

void ms_printf(const char* format, ...);

void ms_printf_insert(const char* format, ...);

void ms_init(int (*write_fun)(const char*, size_t));

void ms_input(char data);

void ms_dir_init(ms_item_t* dir, const char* name);

void ms_file_init(ms_item_t* file, const char* name, ms_cmd_fun_t run_fun,
                  void* data, size_t size, bool writeable);

void ms_dev_init(ms_item_t* file, const char* name, ms_write_fun_t write_fun,
                 ms_read_fun_t read_fun);

void ms_item_add(ms_item_t* item, ms_item_t* parent_dir);

void ms_cmd_add(ms_item_t* cmd);

void ms_enter();

void ms_clear();

void ms_clear_line();

void ms_show_head();

ms_item_t* ms_get_root_dir();

ms_item_t* ms_get_etc_dir();

ms_item_t* ms_get_dev_dir();

ms_item_t* ms_get_bin_dir();

ms_item_t* ms_get_home_dir();

ms_item_t* ms_get_userhome_dir();

void ms_start();

ms_status_t ms_path_to_file(const char* path, ms_item_t** ans);

ms_status_t ms_path_to_dir(const char* raw_path, ms_item_t** ans);

#ifdef __cplusplus
}
#endif

#endif
