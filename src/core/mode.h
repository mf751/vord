#ifndef MODE_H
#define MODE_H

typedef enum {
  NORMAL_MODE = 0,
  INSERT_MODE,
  ADDRESS_MODE,
  VISUAL_MODE,
} EditorMode;

char *get_mode_name();

EditorMode get_mode();

void set_mode(EditorMode new_mode);

#endif // !MODE_H
