// Written by Jacob Rusch
// 12/5/2016
// code for Digital Watch with weather and health
// Supports DIORITE


#include <pebble.h>
#include "watchface.h"


static Window *s_main_window;
static Layer *s_main_layer, *s_battery_layer, *s_health_layer;
static TextLayer *s_clock_layer, *s_date_layer, *s_day_layer, *s_temp_layer;
static GBitmap *s_weather_bitmap, *s_bluetooth_bitmap;
static BitmapLayer *s_weather_bitmap_layer, *s_bluetooth_bitmap_layer;
static int battery_percent, step_goal=100, line_width=6;
static GFont s_clock_font, s_word_font;
static char icon_buf[64];
static double step_count;
static bool charging;
static ClaySettings settings; // An instance of the struct



////////////////////////////////
// clay settings              //
// configures user's settings //
////////////////////////////////
//**********************************************************************************************************************//



///////////////////////////////
// set default Clay settings //
///////////////////////////////
static void config_default() {
	settings.BackgroundColor = GColorWhite;
  settings.ForegroundColor = GColorBlack;
  settings.InvertColors = false;
}


/////////////////////////////////////
// load default settings from Clay //
/////////////////////////////////////
static void config_load() {
	config_default(); // Load the default settings
	persist_read_data(SETTINGS_KEY, &settings, sizeof(settings));	// Read settings from persistent storage, if they exist
  
  APP_LOG(APP_LOG_LEVEL_DEBUG, "config_load");
}


///////////////////////
// sets watch colors //
///////////////////////
static void set_colors() {
  
  // set background color
  window_set_background_color(s_main_window, settings.BackgroundColor);
  
  // set text color for TextLayers
  text_layer_set_text_color(s_clock_layer, settings.ForegroundColor);
  text_layer_set_text_color(s_date_layer, settings.ForegroundColor);
  text_layer_set_text_color(s_day_layer, settings.ForegroundColor);
  text_layer_set_text_color(s_temp_layer, settings.ForegroundColor);
    
  // load appropriate icon
  load_icons(); 
  
  APP_LOG(APP_LOG_LEVEL_DEBUG, "set_colors");
}


/////////////////////////
// saves user settings //
/////////////////////////
static void config_save() {
  persist_write_data(SETTINGS_KEY, &settings, sizeof(settings));
  
  APP_LOG(APP_LOG_LEVEL_DEBUG, "config_save");
}


//////////////////////////////////////////////////////
// display appropriate weather icon                 //
// works with DarkSky.net and OpenWeatherMap.org    //
// https://darksky.net/dev/docs/response#data-point //
// https://openweathermap.org/weather-conditions    //
//////////////////////////////////////////////////////
static void load_icons() {
  
  // if inverted
  if(!settings.InvertColors) {
    // populate icon variable
    
    // DS clear-day
    // OW 01d (clear sky, day)
    
    if(strcmp(icon_buf, "clear-day")==0 || 
       strcmp(icon_buf, "01d")==0) {
      s_weather_bitmap = gbitmap_create_with_resource(RESOURCE_ID_CLEAR_SKY_DAY_BLACK_ICON);  
      
    // DS clear-night
    // OW 01n (clear sky, night)
      
    } else if(strcmp(icon_buf, "clear-night")==0 || 
              strcmp(icon_buf, "01n")==0) {
      s_weather_bitmap = gbitmap_create_with_resource(RESOURCE_ID_CLEAR_SKY_NIGHT_BLACK_ICON);
      
    // DS rain
    // OW 09d (shower rain, day)
    // OW 09n (shower rain, night)
    // OW 10d (rain, day)
    // OW 10n (rain, night)
    // OW 11d (thunderstorm, day)
    // OW 11n (thunderstorm, night)
      
    } else if(strcmp(icon_buf, "rain")==0 ||
             strcmp(icon_buf, "09d")==0 || 
             strcmp(icon_buf, "09n")==0 || 
             strcmp(icon_buf, "10d")==0 || 
             strcmp(icon_buf, "10n")==0 || 
             strcmp(icon_buf, "11d")==0 || 
             strcmp(icon_buf, "11n")==0) {
      s_weather_bitmap = gbitmap_create_with_resource(RESOURCE_ID_RAIN_BLACK_ICON);
      
    // OW 50d (mist, day)
      
    } else if(strcmp(icon_buf, "50d")==0) {
      s_weather_bitmap = gbitmap_create_with_resource(RESOURCE_ID_MIST_DAY_BLACK_ICON);
      
    // OW 50n (mist, night)
      
    } else if(strcmp(icon_buf, "50n")==0) {
      s_weather_bitmap = gbitmap_create_with_resource(RESOURCE_ID_MIST_NIGHT_BLACK_ICON);
      
    // DS snow
    // OW 13d (snow, day)
    // OW 13n (snow, night)
      
    } else if(strcmp(icon_buf, "snow")==0 || 
              strcmp(icon_buf, "13d")==0 || 
              strcmp(icon_buf, "13n")==0) {
      s_weather_bitmap = gbitmap_create_with_resource(RESOURCE_ID_SNOW_BLACK_ICON);
      
    // DS sleet
      
    } else if(strcmp(icon_buf, "sleet")==0) {
      s_weather_bitmap = gbitmap_create_with_resource(RESOURCE_ID_SLEET_BLACK_ICON);
      
    // DS wind
      
    } else if(strcmp(icon_buf, "wind")==0) {
      s_weather_bitmap = gbitmap_create_with_resource(RESOURCE_ID_WIND_BLACK_ICON);
      
    // DS fog
      
    } else if(strcmp(icon_buf, "fog")==0) {
      s_weather_bitmap = gbitmap_create_with_resource(RESOURCE_ID_FOG_BLACK_ICON);
      
    // DS cloudy
      
    } else if(strcmp(icon_buf, "cloudy")==0) {
      s_weather_bitmap = gbitmap_create_with_resource(RESOURCE_ID_CLOUDY_BLACK_ICON);
      
    // DS partly-cloudy-day
    // OW 02d (few clouds, day)
    // OW 03d (scattered clouds, day)
    // OW 04d (broken clouds, day)
      
    } else if(strcmp(icon_buf, "partly-cloudy-day")==0 || 
              strcmp(icon_buf, "02d")==0 || 
              strcmp(icon_buf, "03d")==0 || 
              strcmp(icon_buf, "04d")==0) {
      s_weather_bitmap = gbitmap_create_with_resource(RESOURCE_ID_PARTLY_CLOUDY_DAY_BLACK_ICON);
      
    // DS partly-cloudy-night
    // OW 02d (few clouds, night)
    // OW 03d (scattered clouds, night)
    // OW 04d (broken clouds, night)      
      
    } else if(strcmp(icon_buf, "partly-cloudy-night")==0 || 
              strcmp(icon_buf, "02n")==0 || 
              strcmp(icon_buf, "03n")==0 || 
              strcmp(icon_buf, "04n")==0) {
      s_weather_bitmap = gbitmap_create_with_resource(RESOURCE_ID_PARTLY_CLOUDY_NIGHT_BLACK_ICON);
    } 
    
  } else {
  // not inverted
  // populate icon variable
    
    // DS clear-day
    // OW 01d (clear sky, day)    
    
    if(strcmp(icon_buf, "clear-day")==0 || 
       strcmp(icon_buf, "01d")==0) {
      s_weather_bitmap = gbitmap_create_with_resource(RESOURCE_ID_CLEAR_SKY_DAY_WHITE_ICON);  
      
    // DS clear-night
    // OW 01n (clear sky, night)
      
    } else if(strcmp(icon_buf, "clear-night")==0 || 
              strcmp(icon_buf, "01n")==0) {
      s_weather_bitmap = gbitmap_create_with_resource(RESOURCE_ID_CLEAR_SKY_NIGHT_WHITE_ICON);
      
    // DS rain
    // OW 09d (shower rain, day)
    // OW 09n (shower rain, night)
    // OW 10d (rain, day)
    // OW 10n (rain, night)
    // OW 11d (thunderstorm, day)
    // OW 11n (thunderstorm, night)
      
    } else if(strcmp(icon_buf, "rain")==0 ||
             strcmp(icon_buf, "09d")==0 || 
             strcmp(icon_buf, "09n")==0 || 
             strcmp(icon_buf, "10d")==0 || 
             strcmp(icon_buf, "10n")==0 || 
             strcmp(icon_buf, "11d")==0 || 
             strcmp(icon_buf, "11n")==0) {
      s_weather_bitmap = gbitmap_create_with_resource(RESOURCE_ID_RAIN_WHITE_ICON);
      
    // OW 50d (mist, day)
      
    } else if(strcmp(icon_buf, "50d")==0) {
      s_weather_bitmap = gbitmap_create_with_resource(RESOURCE_ID_MIST_DAY_WHITE_ICON);
      
    // OW 50n (mist, night)
      
    } else if(strcmp(icon_buf, "50n")==0) {
      s_weather_bitmap = gbitmap_create_with_resource(RESOURCE_ID_MIST_NIGHT_WHITE_ICON);      
      
    // DS snow
    // OW 13d (snow, day)
    // OW 13n (snow, night)
      
    } else if(strcmp(icon_buf, "snow")==0 || 
              strcmp(icon_buf, "13d")==0 || 
              strcmp(icon_buf, "13n")==0) {
      s_weather_bitmap = gbitmap_create_with_resource(RESOURCE_ID_SNOW_WHITE_ICON);
      
    // DS sleet
      
    } else if(strcmp(icon_buf, "sleet")==0) {
      s_weather_bitmap = gbitmap_create_with_resource(RESOURCE_ID_SLEET_WHITE_ICON);
      
    // DS wind
      
    } else if(strcmp(icon_buf, "wind")==0) {
      s_weather_bitmap = gbitmap_create_with_resource(RESOURCE_ID_WIND_WHITE_ICON);
      
    // DS fog
      
    } else if(strcmp(icon_buf, "fog")==0) {
      s_weather_bitmap = gbitmap_create_with_resource(RESOURCE_ID_FOG_WHITE_ICON);
      
    // DS cloudy
      
    } else if(strcmp(icon_buf, "cloudy")==0) {
      s_weather_bitmap = gbitmap_create_with_resource(RESOURCE_ID_CLOUDY_WHITE_ICON);
      
    // DS partly-cloudy-day
    // OW 02d (few clouds, day)
    // OW 03d (scattered clouds, day)
    // OW 04d (broken clouds, day)
      
    } else if(strcmp(icon_buf, "partly-cloudy-day")==0 || 
              strcmp(icon_buf, "02d")==0 || 
              strcmp(icon_buf, "03d")==0 || 
              strcmp(icon_buf, "04d")==0) {
      s_weather_bitmap = gbitmap_create_with_resource(RESOURCE_ID_PARTLY_CLOUDY_DAY_WHITE_ICON);
      
    // DS partly-cloudy-night
    // OW 02d (few clouds, night)
    // OW 03d (scattered clouds, night)
    // OW 04d (broken clouds, night)
      
    } else if(strcmp(icon_buf, "partly-cloudy-night")==0 || 
              strcmp(icon_buf, "02n")==0 || 
              strcmp(icon_buf, "03n")==0 || 
              strcmp(icon_buf, "04n")==0) {
      s_weather_bitmap = gbitmap_create_with_resource(RESOURCE_ID_PARTLY_CLOUDY_NIGHT_WHITE_ICON);
    }   
  }
  
  // populate weather icon
  if(s_weather_bitmap_layer) {
    bitmap_layer_destroy(s_weather_bitmap_layer);
  }
  s_weather_bitmap_layer = bitmap_layer_create(GRect(36, 24, 24, 24));
  bitmap_layer_set_compositing_mode(s_weather_bitmap_layer, GCompOpSet);
  bitmap_layer_set_bitmap(s_weather_bitmap_layer, s_weather_bitmap);
  layer_add_child(s_main_layer, bitmap_layer_get_layer(s_weather_bitmap_layer));
}



//////////////////
// update procs //
//////////////////
//**********************************************************************************************************************//



///////////////////////////
// update battery status //
// horizontal            //
///////////////////////////
static void battery_update_proc(Layer *layer, GContext *ctx) {
//   graphics_context_set_fill_color(ctx, settings.ForegroundColor);
//   graphics_context_set_stroke_color(ctx, settings.ForegroundColor);
//   // draw horizontal battery icon (dynamic)
//   int start_x = 127;
//   int start_y = 2;
//   int batt_width = 3; // 3 is slim, 5 is fat
//   int batt = battery_percent/10;
//   graphics_draw_round_rect(ctx, GRect(start_x, start_y, 14, batt_width+4), 1);
//   graphics_fill_rect(ctx, GRect(start_x+2, start_y+2, batt, batt_width), 1, GCornerNone);
//   graphics_fill_rect(ctx, GRect(start_x+14, start_y+2, 1, batt_width), 0, GCornerNone); 
  GRect bounds = layer_get_bounds(layer);
  graphics_context_set_stroke_color(ctx, settings.ForegroundColor);
  graphics_context_set_stroke_width(ctx, line_width);
  // dynamically draw line across bottom
  graphics_fill_rect(ctx, GRect(0, 0, battery_percent*.01*bounds.size.w, line_width), 0, GCornerNone);
  
}


//////////////////////////
// update health status //
//////////////////////////
static void health_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  graphics_context_set_stroke_color(ctx, settings.ForegroundColor);
  graphics_context_set_stroke_width(ctx, line_width);
  // dynamically draw line across bottom
  graphics_fill_rect(ctx, GRect(0, bounds.size.h-line_width, (step_count/step_goal)*bounds.size.w, line_width), 0, GCornerNone);
//   graphics_draw_line(ctx, GPoint(0, bounds.size.h-line_width+1), GPoint((step_count/step_goal)*bounds.size.w, bounds.size.h-line_width+1));
}



//////////////
// handlers //
//////////////
//**********************************************************************************************************************//



//////////////////
// handle ticks //
//////////////////
static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
  
  // Get weather update every 30 minutes
  if(tick_time->tm_min % 30 == 0) {
    // Begin dictionary
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);
  
    // Add a key-value pair
    dict_write_uint8(iter, 0, 0);
  
    // Send the message!
    app_message_outbox_send();
  }  
}


///////////////////////
// update clock time //
///////////////////////
static void update_time() {
  // get a tm strucutre
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  
  // write the hour into a buffer
  static char s_time_buffer[16];
  strftime(s_time_buffer, sizeof(s_time_buffer), clock_is_24h_style() ? "%H:%M" : "%I:%M", tick_time);
//   strftime(s_hour_buffer, sizeof(s_hour_buffer), "%I", tick_time);
  
  // write the minutes into a buffer
  static char s_minute_buffer[8];
  strftime(s_minute_buffer, sizeof(s_minute_buffer), "%M", tick_time);  
  
  // write date to buffer
  static char date_buffer[32];
  strftime(date_buffer, sizeof(date_buffer), "%B %e", tick_time);
  
  // write day to buffer
  static char day_buffer[32];
  char *weekday = day_buffer;
  strftime(day_buffer, sizeof(day_buffer), "%A", tick_time);
  
  // display this time on the text layer
  text_layer_set_text(s_clock_layer, s_time_buffer);
  text_layer_set_text(s_date_layer, date_buffer+(('0' == date_buffer[0]?1:0))); // remove padding
  text_layer_set_text(s_day_layer, weekday); 
}


/////////////////////////////////////
// registers battery update events //
/////////////////////////////////////
static void battery_handler(BatteryChargeState charge_state) {
  battery_percent = charge_state.charge_percent;
  if(charge_state.is_charging || charge_state.is_plugged) {
    charging = true;
  } else {
    charging = false;
  }
  // force update to circle
  layer_mark_dirty(s_battery_layer);
}


// registers health update events
static void health_handler(HealthEventType event, void *context) {
  if(event==HealthEventMovementUpdate) {
    step_count = (double)health_service_sum_today(HealthMetricStepCount);
    layer_mark_dirty(s_health_layer);
  }
  APP_LOG(APP_LOG_LEVEL_INFO, "health_handler completed");
}


/////////////////////////////
// manage bluetooth status //
/////////////////////////////
static void bluetooth_callback(bool connected) {
  layer_set_hidden(bitmap_layer_get_layer(s_bluetooth_bitmap_layer), connected);
  if(!connected) {
    vibes_double_pulse();
  }
}


////////////////////////////
// weather and Clay calls //
////////////////////////////
static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  // Store incoming information
  static char temp_buf[32];

  // Read tuples for data
  Tuple *temp_tuple = dict_find(iterator, MESSAGE_KEY_KEY_TEMP);
  Tuple *icon_tuple = dict_find(iterator, MESSAGE_KEY_KEY_ICON);  
  
  // If all data is available, use it
  if(temp_tuple && icon_tuple) {
    
    // temp
    snprintf(temp_buf, sizeof(temp_buf), "%d°", (int)temp_tuple->value->int32);  
    text_layer_set_text(s_temp_layer, temp_buf);

    // icon
    snprintf(icon_buf, sizeof(icon_buf), "%s", icon_tuple->value->cstring);
  }  
  
  // load weather icons
  load_icons();
  
  // determine if user inverted colors
  Tuple *invert_colors_t = dict_find(iterator, MESSAGE_KEY_KEY_INVERT_COLORS);
  if(invert_colors_t) { settings.InvertColors = invert_colors_t->value->int32 == 1; }
  
  if(settings.InvertColors==1) {
    settings.BackgroundColor = GColorBlack;
    settings.ForegroundColor = GColorWhite;
  } else {
    settings.BackgroundColor = GColorWhite;
    settings.ForegroundColor = GColorBlack;
  }
  
	set_colors();	
	config_save();
  
  APP_LOG(APP_LOG_LEVEL_INFO, "inbox_received_callback");
}


static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}


static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}


static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}



////////////////////////////////
// main functions             //
////////////////////////////////
//**********************************************************************************************************************//



//////////////////////
// load main window //
//////////////////////
static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
  // fonts
  s_word_font = fonts_load_custom_font(resource_get_handle(WORD_FONT));
  s_clock_font = fonts_load_custom_font(resource_get_handle(CLOCK_FONT));

  // create main canvas layer
  s_main_layer = layer_create(bounds);
  layer_add_child(window_layer, s_main_layer);
  
  // create battery layer
  s_battery_layer = layer_create(bounds);
  layer_set_update_proc(s_battery_layer, battery_update_proc);
  layer_add_child(s_main_layer, s_battery_layer);    
  
  // temperature layer
  s_temp_layer = text_layer_create(GRect(bounds.size.w/2, 26, bounds.size.w/2-14, 16));
  text_layer_set_background_color(s_temp_layer, GColorClear);
  text_layer_set_text_alignment(s_temp_layer, GTextAlignmentCenter);
  text_layer_set_font(s_temp_layer, s_word_font);
  layer_add_child(s_main_layer, text_layer_get_layer(s_temp_layer));      
  
  // clock layer
  s_clock_layer = text_layer_create(GRect(0, 50, bounds.size.w, 62));
  text_layer_set_background_color(s_clock_layer, GColorClear);
  text_layer_set_text_alignment(s_clock_layer, GTextAlignmentCenter);
  text_layer_set_font(s_clock_layer, s_clock_font);
  layer_add_child(s_main_layer, text_layer_get_layer(s_clock_layer));    
  
  // date
  s_date_layer = text_layer_create(GRect(0, 114, bounds.size.w, 16));
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
  text_layer_set_font(s_date_layer, s_word_font);
  layer_add_child(s_main_layer, text_layer_get_layer(s_date_layer));  
  
  // day of week
  s_day_layer = text_layer_create(GRect(0, 130, bounds.size.w, 16));
  text_layer_set_background_color(s_day_layer, GColorClear);
  text_layer_set_text_alignment(s_day_layer, GTextAlignmentCenter);
  text_layer_set_font(s_day_layer, s_word_font);
  layer_add_child(s_main_layer, text_layer_get_layer(s_day_layer));   
  
  // draw health line across bottom (dynamic)
  s_health_layer = layer_create(bounds);
  layer_set_update_proc(s_health_layer, health_update_proc);
  layer_add_child(s_main_layer, s_health_layer);  
  
  // manage bluetooth
  if(s_bluetooth_bitmap_layer) {
    bitmap_layer_destroy(s_bluetooth_bitmap_layer);
  }
  if(!settings.InvertColors) {
    s_bluetooth_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BLUETOOTH_DISCONNECTED_BLACK_ICON);  
  } else {
    s_bluetooth_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BLUETOOTH_DISCONNECTED_WHITE_ICON);
  }
  // bluetooth icon
  s_bluetooth_bitmap_layer = bitmap_layer_create(GRect(118, 1, 9, 10));
  bitmap_layer_set_compositing_mode(s_bluetooth_bitmap_layer, GCompOpSet);
  bitmap_layer_set_bitmap(s_bluetooth_bitmap_layer, s_bluetooth_bitmap);
  layer_add_child(s_main_layer, bitmap_layer_get_layer(s_bluetooth_bitmap_layer));     
 
	set_colors();	
	config_save(); 
  
  APP_LOG(APP_LOG_LEVEL_DEBUG, "main_window_load");
}


///////////////////
// unload window //
///////////////////
static void main_window_unload(Window *window) {
  layer_destroy(s_main_layer);
  layer_destroy(s_battery_layer);
  layer_destroy(s_health_layer);
  
  text_layer_destroy(s_clock_layer);
  text_layer_destroy(s_date_layer);
  text_layer_destroy(s_day_layer);
  text_layer_destroy(s_temp_layer);  
  
  gbitmap_destroy(s_weather_bitmap);
  gbitmap_destroy(s_bluetooth_bitmap);
  
  bitmap_layer_destroy(s_weather_bitmap_layer);
  bitmap_layer_destroy(s_bluetooth_bitmap_layer);
}


////////////////////
// initialize app //
////////////////////
static void init() {
  config_load();
  
  s_main_window = window_create();
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  
  // show window on the watch with animated=true
  window_stack_push(s_main_window, true);
  
  // subscribe to time events
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  
  // Make sure the time is displayed from the start
  update_time();
    
  // subscribe to health events 
  health_service_events_subscribe(health_handler, NULL); 
  // force initial update
  health_handler(HealthEventMovementUpdate, NULL);
    
  // register with Battery State Service
  battery_state_service_subscribe(battery_handler);
  // force initial update
  battery_handler(battery_state_service_peek());      
  
  // register with bluetooth state service
  connection_service_subscribe((ConnectionHandlers) {
    .pebble_app_connection_handler = bluetooth_callback
  });
  bluetooth_callback(connection_service_peek_pebble_app_connection());  
    
  // Register weather callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);  
  app_message_open(128, 128);  
  
  APP_LOG(APP_LOG_LEVEL_DEBUG, "init");  
}


///////////////////////
// de-initialize app //
///////////////////////
static void deinit() {
  window_destroy(s_main_window);
}


/////////////
// run app //
/////////////
int main(void) {
  init();
  app_event_loop();
  deinit();
}