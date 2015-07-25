#include "pebble.h"

#define NUM_PATHS 2

// This defines graphics path information to be loaded as a path later
// static const GPathInfo HOUSE_PATH_POINTS = {
// 17,
// (GPoint []) {{-37, 66}, {43,66}, {43,49}, {13,49}, {13,7}, {54,7}, {54,-11}, {12,-11}, {12,-35}, {-10,-35}, {-10,-13}, {-53,-13}, {-53,5}, {-9,5}, {-8,47}, {-38,47}, {-38,64}, }
// };

  static const GPathInfo HOUSE_PATH_POINTS = {
14,
(GPoint []) {{41, -6}, {45,-31}, {32,-16}, {20,-18}, {16,-30}, {29,-45}, {4,-37}, {0,-10}, {-33,28}, {-32,42}, {-18,41}, {15,3}, {41,-4}, {41,-4}, }
};


// This is an example of another super simple shape, 
// notice that negative y coordinates move the drawing point
// above the center of the watchface.

static const GPathInfo INFINITY_RECT_PATH_POINTS = {
20,
(GPoint []) {{-48, -39}, {50,-39}, {51,-8}, {42,18}, {32,35}, {18,45}, {-5,54}, {-21,54}, {-36,54}, {-23,41}, {-4,41}, {10,32}, {21,24}, {31,10}, {37,-9}, {36,-24}, {-30,-24}, {-31,-11}, {-47,-19}, {-48,-39}, }
};


static Window *s_main_window;
static Layer *s_path_layer;

static GPath *s_path_array[NUM_PATHS];
static GPath *s_house_path, *s_infinity_path, *s_current_path;

static int s_current_path_index;
static int s_path_angle;
static bool s_outline_mode;



// This is the layer update callback which is called on render updates
static void path_layer_update_callback(Layer *layer, GContext *ctx) {
  // You can rotate the path before rendering
  gpath_rotate_to(s_current_path, (TRIG_MAX_ANGLE / 360) * s_path_angle);

  // There are two ways you can draw a GPath: outline or filled
  // In this example, only one or the other is drawn, but you can draw
  // multiple instances of the same path filled or outline.
  if (s_outline_mode) {
    // draw outline uses the stroke color
    graphics_context_set_stroke_color(ctx, GColorWhite);
    gpath_draw_outline(ctx, s_current_path);
  } else {
    // draw filled uses the fill color
    graphics_context_set_fill_color(ctx, GColorWhite);
    gpath_draw_filled(ctx, s_current_path);
  }
}

static int path_angle_add() {
  return s_path_angle = (s_path_angle + 10) % 360;
}
static int path_angle_remove() {
  return s_path_angle = (s_path_angle - 10) % 360;
}

void app_timer_handler_add(){
  path_angle_add();
  layer_mark_dirty(s_path_layer);
  app_timer_register(10, app_timer_handler_add, NULL);
  
}
void app_timer_handler_remove(){
  path_angle_remove();
  layer_mark_dirty(s_path_layer);
  app_timer_register(10, app_timer_handler_remove, NULL);
  
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  // Rotate the path counter-clockwise
  path_angle_add();
  layer_mark_dirty(s_path_layer);
  app_timer_handler_add();
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  // Rotate the path clockwise
  path_angle_remove();
  layer_mark_dirty(s_path_layer);
  app_timer_handler_remove();

 }

static void select_raw_down_handler(ClickRecognizerRef recognizer, void *context) {
  // Show the outline of the path when select is held down
  s_outline_mode = true;
  layer_mark_dirty(s_path_layer);
}
static void select_raw_up_handler(ClickRecognizerRef recognizer, void *context) {
  // Show the path filled
  s_outline_mode = false;

  // Cycle to the next path
  s_current_path_index = (s_current_path_index + 1) % NUM_PATHS;
  s_current_path = s_path_array[s_current_path_index];
  layer_mark_dirty(s_path_layer);
}



static void config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
  window_raw_click_subscribe(BUTTON_ID_SELECT, select_raw_down_handler, select_raw_up_handler, NULL);
}

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);

  s_path_layer = layer_create(bounds);
  layer_set_update_proc(s_path_layer, path_layer_update_callback);
  layer_add_child(window_layer, s_path_layer);

  // Move all paths to the center of the screen
  for (int i = 0; i < NUM_PATHS; i++) {
    gpath_move_to(s_path_array[i], GPoint(bounds.size.w/2, bounds.size.h/2));
  }
}

static void main_window_unload(Window *window) {
  layer_destroy(s_path_layer);
}

static void init() {
  // Pass the corresponding GPathInfo to initialize a GPath
  s_house_path = gpath_create(&HOUSE_PATH_POINTS);
  s_infinity_path = gpath_create(&INFINITY_RECT_PATH_POINTS);

  // This demo allows you to cycle paths in an array
  // Try adding more GPaths to cycle through
  // You'll need to define another GPathInfo
  // Remember to update NUM_PATHS accordingly
  s_path_array[0] = s_house_path;
  s_path_array[1] = s_infinity_path;

  s_current_path = s_path_array[0];

  // Create Window
  s_main_window = window_create();
  window_set_background_color(s_main_window, GColorBlack);
  window_set_click_config_provider(s_main_window, config_provider);
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload,
  });
  window_stack_push(s_main_window, true);
}

static void deinit() {
  window_destroy(s_main_window);

  gpath_destroy(s_house_path);
  gpath_destroy(s_infinity_path);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
