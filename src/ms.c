#include "ms.h"

#define SHELL_VERSION "1.0.6"

static const char INIT_MESSAGE[] =
    " __  __ _      _ ___ _        _ _ \r\n"
    "|  \\/  (_)_ _ (_) __| |_  ___| | |\r\n"
    "| |\\/| | | ' \\| \\__ \\ ' \\/ -_) | |\r\n"
    "|_|  |_|_|_||_|_|___/_||_\\___|_|_|\r\n"
    "Build:"__DATE__
    " "__TIME__
    "\r\n"
    "version:" SHELL_VERSION "\r\n";

static const char CLEAR_ALL[] = "\033[2J\033[1H";
static const char CLEAR_LINE[] = "\033[2K\r";
static const char CLEAR_BEHIND[] = "\033[K";
static const char KEY_RIGHT[] = "\033[C";
static const char KEY_LEFT[] = "\033[D";
static const char KEY_SAVE[] = "\033[s";
static const char KEY_LOAD[] = "\033[u";

#define MS_HISTORY_NULL (0X100)

ms_t ms;

static void err_arg_num() {
  ms_printf("ERROR: Invalid number of arguments.");
  ms_enter();
}

static void err_file_write() {
  ms_printf("ERROR: File not writable.");
  ms_enter();
}

static void err_file_read() {
  ms_printf("ERROR: File not readable.");
  ms_enter();
}

static void err_file_found(const char* name) {
  if (*name == '\0') {
    ms_printf("ERROR: File not found.", name);
  } else {
    ms_printf("ERROR: File '%s' not found.", name);
  }
  ms_enter();
}

static void err_file_exec() {
  ms_printf("File not executable.");
  ms_enter();
}

static void err_dir_found(const char* name) {
  if (*name == '\0') {
    ms_printf("ERROR: Directory not found.", name);
  } else {
    ms_printf("ERROR: Directory '%s' not found.", name);
  }
  ms_enter();
}

static char* remove_path(char* cmd) {
  char* cmd_name_index = strrchr(cmd, '/');
  if (!cmd_name_index) {
    return cmd;
  }

  if (*(cmd_name_index + 1) != '\0') {
    *cmd_name_index = '\0';
  }

  return cmd_name_index + 1;
}

static ms_item_t* path_to_dir(char* path) {
  size_t dir_name_len = strlen(path);
  ms_item_t* target = ms.ctrl.cur_dir;

  if (dir_name_len == 0) {
    return target;
  }

  if (path[0] == '/') {
    target = &ms.sys_file.root_dir;
    path++;
    dir_name_len--;
    if (dir_name_len == 0) {
      return target;
    }
  } else if (path[dir_name_len - 1] == '/') {
    path[dir_name_len - 1] = '\0';
    dir_name_len--;
  }

  size_t index = 0;

  for (size_t i = 0; i < dir_name_len + 1; i++) {
    if (path[i] != '/' && path[i] != '\0') {
      continue;
    }

    path[i] = '\0';

    if (strcmp("..", path + index) == 0) {
      if (target != &ms.sys_file.root_dir) {
        target = target->father;
      }
      index = i + 1;
      path[i] = '/';
      continue;
    }

    if (strcmp(".", path + index) == 0) {
      index = i + 1;
      path[i] = '/';
      continue;
    }

    om_list_head_t* pos = NULL;

    bool found = false;

    om_list_for_each(pos, &target->data.as_dir.list) {
      ms_item_t* item = om_list_entry(pos, ms_item_t, self);

      if (item->mode == MS_MODE_DIR && strcmp(path + index, item->name) == 0) {
        target = item;
        index = i + 1;
        found = true;
        path[i] = '/';
        break;
      }
    }

    if (found) {
      continue;
    }

    return NULL;
  }

  return target;
}

static ms_item_t* path_to_file(char* raw) {
  char* cmd_name = remove_path(raw);

  ms_item_t* search_dir = NULL;
  bool search_bin = false;

  if (cmd_name == raw) {
    search_dir = ms.ctrl.cur_dir;
    if (ms.ctrl.cur_dir != &ms.sys_file.bin_dir) {
      search_bin = true;
    }
  } else if (!(search_dir = path_to_dir(raw))) {
    err_dir_found(raw);
    return NULL;
  }

  om_list_head_t* pos = NULL;

  om_list_for_each(pos, &search_dir->data.as_dir.list) {
    ms_item_t* item = om_list_entry(pos, ms_item_t, self);
    if (item->mode != MS_MODE_DIR && strcmp(cmd_name, item->name) == 0) {
      return item;
    }
  }

  if (search_bin) {
    om_list_for_each(pos, &ms.sys_file.bin_dir.data.as_dir.list) {
      ms_item_t* item = om_list_entry(pos, ms_item_t, self);
      if (item->mode != MS_MODE_DIR && strcmp(cmd_name, item->name) == 0) {
        return item;
      }
    }
  }

  err_file_found(raw);

  return NULL;
}

#if MS_CMD_PWD
static void pwd_fun_fn(ms_item_t* item) {
  if (item == &ms.sys_file.root_dir) {
    return;
  }

  if (item->father) {
    pwd_fun_fn(item->father);
  }
  ms_printf("/%s", item->name);
}

static int pwd_fun(ms_item_t* item, int argc, char* argv[]) {
  OM_UNUSED(argc);
  OM_UNUSED(argv);
  OM_UNUSED(item);

  if (ms.ctrl.cur_dir == &ms.sys_file.root_dir) {
    ms.write("/", 1);
  } else {
    pwd_fun_fn(ms.ctrl.cur_dir);
  }
  ms_enter();
  return 0;
}
#endif

#if MS_CMD_LS
static int ls_fun(ms_item_t* _item, int argc, char* argv[]) {
  OM_UNUSED(_item);

  ms_item_t* item = NULL;
  if (argc == 1) {
    item = ms.ctrl.cur_dir;
  } else if (argc == 2) {
    item = path_to_dir(argv[1]);
    if (!item) {
      err_dir_found(argv[1]);
      return -1;
    }
  }

  om_list_head_t* pos = NULL;
  om_list_for_each(pos, &item->data.as_dir.list) {
    ms_item_t* item = om_list_entry(pos, ms_item_t, self);

    switch (item->mode) {
      case MS_MODE_DIR:
        ms_printf("d---   %-15s", item->name);
        break;
      case MS_MODE_FILE: {
        if (item->data.as_file.data) {
          ms_printf(
              "f%c%c%c   %-15s size %d", item->data.as_file.run ? 'x' : '-',
              item->data.as_file.data ? 'r' : '-',
              item->data.as_file.data && item->data.as_file.writeable ? 'w'
                                                                      : '-',
              item->name, item->data.as_file.size);
          break;
        } else {
          ms_printf("f%c--   %-15s", item->data.as_file.run ? 'x' : '-',
                    item->name, item->data.as_file.size);
          break;
        }
      }
      case MS_MODE_DEV:
        ms_printf("c-%c%c   %-15s", item->data.as_device.read ? 'r' : '-',
                  item->data.as_device.write ? 'w' : '-', item->name);
        break;
    }

    ms_enter();
  }

  return 0;
}
#endif

#if MS_CMD_LS
static int cd_fun(ms_item_t* item, int argc, char* argv[]) {
  OM_UNUSED(item);

  if (argc == 1) {
    ms.ctrl.cur_dir = &ms.sys_file.user_home_dir;
    return 0;
  }

  if (argc != 2) {
    err_arg_num();
    return -1;
  };

  ms_item_t* dir = path_to_dir(argv[1]);

  if (dir) {
    ms.ctrl.cur_dir = dir;
    return 0;
  } else {
    err_dir_found(argv[1]);
    return -1;
  }
}
#endif

#if MS_CMD_CAT
static int cat_fun(ms_item_t* _item, int argc, char* argv[]) {
  OM_UNUSED(_item);

  if (argc != 2) {
    err_arg_num();
    return -1;
  };

  ms_item_t* item = path_to_file(argv[1]);

  if (!item) {
    return -1;
  }

  switch (item->mode) {
    case MS_MODE_FILE: {
      if (!item->data.as_file.data) {
        err_file_read();
        return -1;
      }

      size_t count = strnlen(item->data.as_file.data, item->data.as_file.size);

      ms.write(item->data.as_file.data, count);

      ms_enter();
      break;
    }

    case MS_MODE_DEV:
      if (!item->data.as_device.read) {
        err_file_read();
        return -1;
      }

      int count = 0;

      while (1) {
        count =
            item->data.as_device.read(item, ms.buff.cat_buff, MS_CAT_BUFF_SIZE);

        if (count <= 0) {
          return -1;
        }

        ms.write(ms.buff.cat_buff, count);

        if (count != MS_CAT_BUFF_SIZE) {
          break;
        }
      }
      break;
    default:
      return -1;
  }

  return 0;
}
#endif

#if MS_CMD_ECHO
static int echo_fun(ms_item_t* item, int argc, char* argv[]) {
  OM_UNUSED(item);

  if (argc == 2) {
    ms_printf("%s", argv[1]);
    ms_enter();
    return 0;
  } else if (argc == 4) {
    if (strcmp(argv[2], ">") != 0) {
      err_arg_num();
      return -1;
    }

    ms_item_t* item = path_to_file(argv[3]);

    if (!item) {
      return -1;
    }

    switch (item->mode) {
      case MS_MODE_FILE: {
        if ((!item->data.as_file.data) || (!item->data.as_file.writeable)) {
          err_file_write();
          return -1;
        }

        size_t count = strnlen(argv[1], MS_MAX_CMD_LENGTH) + 1;

        if (count > item->data.as_file.size) {
          count = item->data.as_file.size;
        }

        memcpy(item->data.as_file.data, argv[1], count);
        break;
      }

      case MS_MODE_DEV: {
        if (!item->data.as_device.write) {
          err_file_write();
          return -1;
        }

        size_t count = strnlen(argv[1], MS_MAX_CMD_LENGTH) + 1;

        item->data.as_device.write(item, argv[1], count);

        break;
      }
      default:
        return -1;
    }

    return 0;
  } else {
    err_arg_num();
    return -1;
  }
}
#endif

#if MS_CMD_CLEAR
static int clear_fun(ms_item_t* item, int argc, char* argv[]) {
  OM_UNUSED(argc);
  OM_UNUSED(argv);
  OM_UNUSED(item);

  ms_clear();

  return 0;
}
#endif

#if MS_FILE_TTY
static int tty_read_fun(ms_item_t* item, void* data, size_t count) {
  OM_UNUSED(item);
  OM_UNUSED(data);
  OM_UNUSED(count);
  ms.write(ms.buff.read_buff, ms.ctrl.index + 1);
  return 0;
}

static int tty_write_fun(ms_item_t* item, const void* data, size_t count) {
  OM_UNUSED(item);

  ms.write(data, count);

  return (int)count;
}
#endif

void ms_enter() { ms.write("\r\n", sizeof("\r\n")); }

static void ms_reset() {
  ms.ctrl.index = 0;
  ms.ctrl.offset = 0;
  ms.ctrl.ansi = 0;
}

void ms_printf(const char* format, ...) {
  va_list v_arg_list;
  va_start(v_arg_list, format);
  (void)vsnprintf(ms.buff.write_buff, sizeof(ms.buff.write_buff), format,
                  v_arg_list);
  va_end(v_arg_list);
  ms.write(ms.buff.write_buff, strlen(ms.buff.write_buff));
}

void ms_printf_insert(const char* format, ...) {
  va_list v_arg_list;
  va_start(v_arg_list, format);
  (void)vsnprintf(ms.buff.write_buff, sizeof(ms.buff.write_buff), format,
                  v_arg_list);
  va_end(v_arg_list);

  ms.write(CLEAR_LINE, sizeof(CLEAR_LINE));
  ms.write(ms.buff.write_buff, strlen(ms.buff.write_buff));
  ms_show_head();
  ms_printf("%s", ms.buff.read_buff);
}

void ms_dir_init(ms_item_t* dir, const char* name) {
  dir->mode = MS_MODE_DIR;
  dir->name = name;
  _OM_INIT_LIST_HEAD(&dir->data.as_dir.list);
}

void ms_file_init(ms_item_t* file, const char* name, ms_cmd_fun_t run_fun,
                  void* data, size_t size, bool writeable) {
  file->mode = MS_MODE_FILE;
  file->name = name;
  file->data.as_file.data = data;
  file->data.as_file.size = size;
  file->data.as_file.writeable = writeable;
  file->data.as_file.run = run_fun;
}

void ms_dev_init(ms_item_t* file, const char* name, ms_write_fun_t write_fun,
                 ms_read_fun_t read_fun) {
  file->mode = MS_MODE_DEV;
  file->name = name;
  file->data.as_device.read = read_fun;
  file->data.as_device.write = write_fun;
}

void ms_item_add(ms_item_t* item, ms_item_t* parent_dir) {
  om_list_add(&item->self, &parent_dir->data.as_dir.list);
  item->father = parent_dir;
}

void ms_cmd_add(ms_item_t* cmd) { ms_item_add(cmd, &ms.sys_file.bin_dir); }

static ms_item_t* ms_search_cmd(char* cmd) {
  ms_item_t* item = path_to_file(cmd);

  if (item) {
    if (item->data.as_file.run) {
      return item;
    } else {
      err_file_exec();
      return NULL;
    }
  }

  return NULL;
}

static char* remove_space(char* cmd) {
  while (ms.ctrl.index > 0 && ms.buff.read_buff[ms.ctrl.index - 1] == ' ') {
    ms.ctrl.index--;
    ms.buff.read_buff[ms.ctrl.index] = '\0';
  }

  while (*cmd == ' ') {
    cmd++;
  }

  return cmd;
}

static void ms_prase_cmd(const char* cmd) {
  if (ms.ctrl.index == 0) {
    return;
  }

  uint8_t arg_num = 0;
  ms.buff.arg_map[0] = ms.buff.prase_buff;

  strncpy(ms.buff.prase_buff, cmd, MS_MAX_CMD_LENGTH);

  for (uint16_t i = 0; ms.buff.prase_buff[i] != '\0'; i++) {
    if (ms.buff.prase_buff[i] == ' ') {
      ms.buff.prase_buff[i] = '\0';
      if (ms.buff.prase_buff[i + 1] != ' ') {
        arg_num++;
        ms.buff.arg_map[arg_num] = ms.buff.prase_buff + i + 1;
      }
    }

    if (arg_num == MS_MAX_ARG_NUM) {
      break;
    }
  }

  ms_item_t* item = ms_search_cmd(ms.buff.prase_buff);

  if (item) {
    item->data.as_file.run(item, arg_num + 1, ms.buff.arg_map);
  }
}

void ms_show_head() {
  if (ms.ctrl.cur_dir == &ms.sys_file.user_home_dir) {
    ms_printf("%s%s@%s:%s$\e[00m ", MS_HEAD_COLOR, MS_USER_NAME, MS_OS_NAME,
              "~");
  } else {
    ms_printf("%s%s@%s:%s$\e[00m ", MS_HEAD_COLOR, MS_USER_NAME, MS_OS_NAME,
              ms.ctrl.cur_dir->name);
  }
}

static void ms_add_history() {
  ms.history.index = MS_HISTORY_NULL;
  strcpy(ms.buff.history_buff[ms.history.last++ % MS_MAX_HISTORY_NUM],
         ms.buff.read_buff);
  ms.history.last %= MS_MAX_HISTORY_NUM;
  ms.history.num++;
  if (ms.history.num >= MS_MAX_HISTORY_NUM) {
    ms.history.num = MS_MAX_HISTORY_NUM;
  }
}

static void ms_apply_history(uint16_t num) {
  ms_clear_line();
  ms_show_head();
  ms.history.index = (int16_t)num;
  ms.ctrl.index = strlen(ms.buff.history_buff[num]);
  strcpy(ms.buff.read_buff, ms.buff.history_buff[num]);
  ms.write(ms.buff.read_buff, ms.ctrl.index);
}

static uint16_t ms_get_next_history() {
  if (ms.history.index == MS_HISTORY_NULL) {
    if (ms.history.num > 0) {
      return (ms.history.last + MS_MAX_HISTORY_NUM - 1) % MS_MAX_HISTORY_NUM;
    } else {
      return MS_HISTORY_NULL;
    }
  } else if (ms.history.num == MS_MAX_HISTORY_NUM) {
    if (ms.history.index != ms.history.last) {
      return (ms.history.index + MS_MAX_HISTORY_NUM - 1) % MS_MAX_HISTORY_NUM;
    } else {
      return MS_HISTORY_NULL;
    }
  } else {
    if (ms.history.index != 0) {
      return ms.history.index - 1;
    } else {
      return MS_HISTORY_NULL;
    }
  }
}

static uint16_t ms_get_prev_history() {
  if (ms.history.index == MS_MAX_HISTORY_NUM) {
    return MS_HISTORY_NULL;

  } else if (ms.history.num == MS_MAX_HISTORY_NUM) {
    if (ms.history.index !=
        ((ms.history.last + MS_MAX_HISTORY_NUM - 1) % MS_MAX_HISTORY_NUM)) {
      return (ms.history.index + 1) % MS_MAX_HISTORY_NUM;
    } else {
      return MS_HISTORY_NULL;
    }
  } else {
    if (ms.history.index != ms.history.last - 1) {
      return ms.history.index + 1;
    } else {
      return MS_HISTORY_NULL;
    }
  }
}

static void ms_key_up() {
  uint16_t history = ms_get_next_history();

  if (history != MS_HISTORY_NULL) {
    ms_apply_history(history);
  }
}

static void ms_key_down() {
  uint16_t history = ms_get_prev_history();

  if (history != MS_HISTORY_NULL) {
    ms_apply_history(history);
  } else {
    ms_clear_line();
    ms_show_head();
  }
}

static void ms_key_left() {
  if (ms.ctrl.index > 0) {
    ms.ctrl.index--;
    ms.ctrl.offset++;
    ms_printf("%s", KEY_LEFT);
  }
}

static void ms_key_right() {
  if (ms.ctrl.offset > 0) {
    ms.ctrl.index++;
    ms.ctrl.offset--;
    ms_printf("%s", KEY_RIGHT);
  }
}

static void ms_prase_ansi(char data) {
  switch (ms.ctrl.ansi) {
    case 1:
      if (isprint((unsigned char)data)) {
        ms.ctrl.ansi++;
        break;
      }
      ms.ctrl.ansi = 0;
      break;
    case 2:
      switch (data) {
        case 'A':
          ms_key_up();
          break;
        case 'B':
          ms_key_down();
          break;
        case 'C':
          ms_key_right();
          break;
        case 'D':
          ms_key_left();
          break;
        default:
          break;
      }
      ms.ctrl.ansi = 0;
      break;
    default:
      break;
  }
}

static void ms_delete() {
  if (ms.ctrl.index > 0) {
    ms_printf("\b \b");
    ms.ctrl.index--;
    ms.buff.read_buff[ms.ctrl.index] = '\0';

    if (ms.ctrl.offset) {
      for (size_t i = 0; i < ms.ctrl.offset; i++) {
        ms.buff.read_buff[ms.ctrl.index + i] =
            ms.buff.read_buff[ms.ctrl.index + i + 1];
      }
      ms.buff.read_buff[ms.ctrl.index + ms.ctrl.offset] = '\0';
      ms_printf("%s%s", KEY_SAVE, CLEAR_BEHIND);
      ms.write(ms.buff.read_buff + ms.ctrl.index, ms.ctrl.offset);
      ms_printf("%s", KEY_LOAD);
    }
  }
}

static void ms_display_char(char data) {
  if (ms.ctrl.index + ms.ctrl.offset < MS_MAX_CMD_LENGTH - 2) {
    for (size_t i = ms.ctrl.offset; i > 0; i--) {
      ms.buff.read_buff[ms.ctrl.index + i] =
          ms.buff.read_buff[ms.ctrl.index + i - 1];
    }

    ms.write(&data, 1);
    ms.buff.read_buff[ms.ctrl.index] = data;
    ms.ctrl.index++;
    if (ms.ctrl.offset) {
      ms.buff.read_buff[ms.ctrl.index + ms.ctrl.offset] = '\0';
      ms_printf("%s%s", KEY_SAVE, CLEAR_BEHIND);
      ms.write(ms.buff.read_buff + ms.ctrl.index, ms.ctrl.offset);
      ms_printf("%s", KEY_LOAD);
    }
  }
}

static void ms_tab_fn(ms_item_t* item, uint16_t counter) {
  if (counter == 1) {
    ms.ctrl.tab_tmp = item;
    ms.ctrl.tab_same_len = 0;
    return;
  }

  if (counter == 2) {
    ms_enter();
    ms_printf("%s", ms.ctrl.tab_tmp->name);
    ms.ctrl.tab_same_len = strlen(ms.ctrl.tab_tmp->name) - 1;
  }

  for (uint16_t i = 0; i <= ms.ctrl.tab_same_len; i++) {
    if (ms.ctrl.tab_tmp->name[i] != item->name[i] || item->name[i] == '\0') {
      ms.ctrl.tab_same_len = i;
      ms.ctrl.tab_tmp = item;
    }
  }

  ms_enter();
  ms_printf("%s", item->name);
}

static void ms_tab(char* cmd) {
  while (*cmd == ' ') {
    cmd++;
  }

  strcpy(ms.buff.prase_buff, cmd);

  char* last_str = strrchr(ms.buff.prase_buff, ' ');

  if (!last_str) {
    last_str = ms.buff.prase_buff;
  } else {
    last_str++;
  }

  const char* name = remove_path(last_str);

  ms_item_t* search_dir = NULL;
  bool search_bin = false;

  if (name == last_str) {
    search_dir = ms.ctrl.cur_dir;
    if (ms.ctrl.cur_dir != &ms.sys_file.bin_dir) {
      search_bin = true;
    }
    if (*name == '\0') {
      search_dir = &ms.sys_file.bin_dir;
      search_bin = false;
    }
  } else {
    search_dir = path_to_dir(last_str);
    if (!search_dir) {
      return;
    }
  }

  if (strcmp(name, "..") == 0 || strcmp(name, ".") == 0) {
    ms_display_char('/');
    return;
  }

  uint16_t counter = 0;

  size_t name_len = strlen(name);

  om_list_head_t* pos = NULL;

  om_list_for_each(pos, &search_dir->data.as_dir.list) {
    ms_item_t* item = om_list_entry(pos, ms_item_t, self);
    if (strncmp(name, item->name, name_len) == 0) {
      counter++;
      ms_tab_fn(item, counter);
    }
  }

  if (search_bin) {
    om_list_for_each(pos, &ms.sys_file.bin_dir.data.as_dir.list) {
      ms_item_t* item = om_list_entry(pos, ms_item_t, self);
      if (strncmp(name, item->name, name_len) == 0) {
        counter++;
        ms_tab_fn(item, counter);
      }
    }
  }

  if (counter == 0) {
    return;
  } else if (counter == 1) {
    if (ms.ctrl.tab_tmp->name[name_len] == '\0' &&
        ms.ctrl.tab_tmp->mode == MS_MODE_DIR) {
      ms_display_char('/');
      return;
    }

    name = ms.ctrl.tab_tmp->name + name_len;
    while (*name) {
      ms_display_char(*name);
      name++;
    }
    return;
  } else if (ms.ctrl.tab_same_len == 0) {
    ms_enter();
    ms_show_head();
    ms_printf("%s", ms.buff.read_buff);
  } else {
    ms_enter();
    ms_show_head();
    ms.write(ms.buff.read_buff, ms.ctrl.index);
    for (uint16_t i = name_len; i < ms.ctrl.tab_same_len; i++) {
      ms_input(ms.ctrl.tab_tmp->name[i]);
    }
  }
}

void ms_input(char data) {
  if (data == '\n' || data == '\r') {
    ms.ctrl.index += ms.ctrl.offset;
    ms_enter();
    ms.buff.read_buff[ms.ctrl.index] = '\0';
    ms_add_history();
    ms_prase_cmd(remove_space(ms.buff.read_buff));
    ms_show_head();
    ms_reset();
    memset(ms.buff.read_buff, 0, MS_MAX_CMD_LENGTH);
    return;
  }

  if (ms.ctrl.ansi) {
    ms_prase_ansi(data);
  } else if (data == '\033') {
    ms.ctrl.ansi = 1;
  } else if (data == 0x7f) {
    ms_delete();
  } else if (data == '\t') {
    ms_tab(ms.buff.read_buff);
  } else if (data == 3) {
    ms_enter();
    ms_clear_line();
    ms_show_head();
  } else {
    ms_display_char(data);
  }
}

void ms_clear_line() {
  ms_reset();
  ms.write(CLEAR_LINE, sizeof(CLEAR_LINE));
}

void ms_clear() {
  ms_reset();
  ms.write(CLEAR_ALL, sizeof(CLEAR_ALL));
}

void ms_init(int (*write_fun)(const char*, size_t)) {
  OM_UNUSED(err_arg_num);
  OM_UNUSED(err_file_write);
  OM_UNUSED(err_file_read);

  ms.write = write_fun;

  ms.history.index = MS_HISTORY_NULL;

  ms_dir_init(&ms.sys_file.root_dir, "/");
  ms_dir_init(&ms.sys_file.bin_dir, "bin");
  ms_dir_init(&ms.sys_file.etc_dir, "etc");
  ms_dir_init(&ms.sys_file.dev_dir, "dev");
  ms_dir_init(&ms.sys_file.home_dir, "home");
  ms_dir_init(&ms.sys_file.user_home_dir, MS_USER_NAME);

  ms_item_add(&ms.sys_file.bin_dir, &ms.sys_file.root_dir);
  ms_item_add(&ms.sys_file.dev_dir, &ms.sys_file.root_dir);
  ms_item_add(&ms.sys_file.etc_dir, &ms.sys_file.root_dir);
  ms_item_add(&ms.sys_file.home_dir, &ms.sys_file.root_dir);
  ms_item_add(&ms.sys_file.user_home_dir, &ms.sys_file.home_dir);
#if MS_CMD_PWD
  ms_file_init(&ms.sys_file.pwd_cmd, "pwd", pwd_fun, NULL, 0, false);
  ms_cmd_add(&ms.sys_file.pwd_cmd);
#endif
#if MS_CMD_LS
  ms_file_init(&ms.sys_file.ls_cmd, "ls", ls_fun, NULL, 0, false);
  ms_cmd_add(&ms.sys_file.ls_cmd);
#endif
#if MS_CMD_CD
  ms_file_init(&ms.sys_file.cd_cmd, "cd", cd_fun, NULL, 0, false);
  ms_cmd_add(&ms.sys_file.cd_cmd);

#endif
#if MS_CMD_CAT
  ms_file_init(&ms.sys_file.cat_cmd, "cat", cat_fun, NULL, 0, false);
  ms_cmd_add(&ms.sys_file.cat_cmd);
#endif
#if MS_CMD_ECHO
  ms_file_init(&ms.sys_file.echo_cmd, "echo", echo_fun, NULL, 0, false);
  ms_cmd_add(&ms.sys_file.echo_cmd);
#endif
#if MS_CMD_CLEAR
  ms_file_init(&ms.sys_file.clear_cmd, "clear", clear_fun, NULL, 0, false);
  ms_cmd_add(&ms.sys_file.clear_cmd);
#endif
#if MS_FILE_TTY
  ms_dev_init(&ms.sys_file.tty_dev, "tty", tty_write_fun, tty_read_fun);
  ms_item_add(&ms.sys_file.tty_dev, &ms.sys_file.dev_dir);
#endif
#if MS_FILE_README
  ms_file_init(&ms.sys_file.readme_file, "README.txt", NULL,
               ms.buff.readme_buff, sizeof(ms.buff.readme_buff), true);
  strcpy(ms.buff.readme_buff,
         "Mini Shell by jiu-xiao. https://github.com/Jiu-xiao/mini_shell");
  ms_item_add(&ms.sys_file.readme_file, &ms.sys_file.user_home_dir);
#endif

  ms.ctrl.cur_dir = &ms.sys_file.user_home_dir;
}

void ms_start() {
  ms_clear();
  const char* index = INIT_MESSAGE;
  for (size_t i = 0; i < sizeof(INIT_MESSAGE); i += 64) {
    if (sizeof(INIT_MESSAGE) - i > 64) {
      ms.write(index, 64);
      index += 64;
    } else {
      ms_printf("%s", index);
    }
  }
  ms_enter();
  ms.write(MS_HELLO_MESSAGE, sizeof(MS_HELLO_MESSAGE));
  ms_enter();
  if (sizeof(MS_INIT_COMMAND) > 1) {
    strncpy(ms.buff.read_buff, MS_INIT_COMMAND, sizeof(MS_INIT_COMMAND));
    ms.ctrl.index = sizeof(MS_INIT_COMMAND);
    ms_input('\n');
  }
  ms_show_head();
}

ms_item_t* ms_get_root_dir() { return &ms.sys_file.root_dir; }

ms_item_t* ms_get_etc_dir() { return &ms.sys_file.etc_dir; }

ms_item_t* ms_get_dev_dir() { return &ms.sys_file.dev_dir; }

ms_item_t* ms_get_bin_dir() { return &ms.sys_file.bin_dir; }

ms_item_t* ms_get_home_dir() { return &ms.sys_file.home_dir; }

ms_item_t* ms_get_userhome_dir() { return &ms.sys_file.user_home_dir; }

ms_status_t ms_path_to_dir(const char* raw_path, ms_item_t** ans) {
  strncpy(ms.buff.path_prase_buff, raw_path, MS_MAX_CMD_LENGTH);
  size_t dir_name_len = strlen(ms.buff.path_prase_buff);
  ms_item_t* target = ms.ctrl.cur_dir;
  char* path = ms.buff.path_prase_buff;

  if (dir_name_len == 0) {
    *ans = target;
    return MS_OK;
  }

  if (path[0] == '/') {
    target = &ms.sys_file.root_dir;
    path++;
    dir_name_len--;
    if (dir_name_len == 0) {
      *ans = target;
      return MS_OK;
    }
  } else if (path[dir_name_len - 1] == '/') {
    path[dir_name_len - 1] = '\0';
    dir_name_len--;
  }

  size_t index = 0;

  for (size_t i = 0; i < dir_name_len + 1; i++) {
    if (path[i] != '/' && path[i] != '\0') {
      continue;
    }

    path[i] = '\0';

    if (strcmp("..", path + index) == 0) {
      if (target != &ms.sys_file.root_dir) {
        target = target->father;
      }
      index = i + 1;
      path[i] = '/';
      continue;
    }

    if (strcmp(".", path + index) == 0) {
      index = i + 1;
      path[i] = '/';
      continue;
    }

    om_list_head_t* pos = NULL;

    bool found = false;

    om_list_for_each(pos, &target->data.as_dir.list) {
      ms_item_t* item = om_list_entry(pos, ms_item_t, self);

      if (item->mode == MS_MODE_DIR && strcmp(path + index, item->name) == 0) {
        target = item;
        index = i + 1;
        found = true;
        path[i] = '/';
        break;
      }
    }

    if (found) {
      continue;
    }

    *ans = NULL;
    return MS_ERROR_NULL;
  }

  *ans = target;
  return MS_OK;
}

ms_status_t ms_path_to_file(const char* raw_path, ms_item_t** ans) {
  strncpy(ms.buff.path_prase_buff, raw_path, MS_MAX_CMD_LENGTH);
  char* path = ms.buff.path_prase_buff;
  char* cmd_name = remove_path(path);

  ms_item_t* search_dir = NULL;
  bool search_bin = false;

  if (cmd_name == path) {
    search_dir = ms.ctrl.cur_dir;
    if (ms.ctrl.cur_dir != &ms.sys_file.bin_dir) {
      search_bin = true;
    }
  } else if (!(search_dir = path_to_dir(path))) {
    *ans = NULL;
    return MS_ERROR_NULL;
  }

  om_list_head_t* pos = NULL;

  om_list_for_each(pos, &search_dir->data.as_dir.list) {
    ms_item_t* item = om_list_entry(pos, ms_item_t, self);
    if (item->mode != MS_MODE_DIR && strcmp(cmd_name, item->name) == 0) {
      *ans = item;
      return MS_OK;
    }
  }

  if (search_bin) {
    om_list_for_each(pos, &ms.sys_file.bin_dir.data.as_dir.list) {
      ms_item_t* item = om_list_entry(pos, ms_item_t, self);
      if (item->mode != MS_MODE_DIR && strcmp(cmd_name, item->name) == 0) {
        *ans = item;
        return MS_OK;
      }
    }
  }

  *ans = NULL;
  return MS_ERROR_NULL;
}
