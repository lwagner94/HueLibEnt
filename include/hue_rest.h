#pragma once

#include "debug.h"



#include "hue_rest.h"

#include <curl/curl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <json-c/json.h>


#define AREA_NAME_LEN       33
#define MAX_LIGHTS_PER_AREA 10

#define HUE_APP_NAME_SIZE 21
#define HUE_DEVICE_NAME_SIZE 20


/* Generic Errors */
#define HUE_ERR_UNAUTHORIZED          1
#define HUE_ERR_INVALID_MSG           2
#define HUE_ERR_NOT_AVAILABLE         3
#define HUE_ERR_INCORRECT_METHOD      4
#define HUE_ERR_MISSING_PARAMS        5
#define HUE_ERR_PARAM_NOT_AVAILABLE   6
#define HUE_ERR_INVALID_VALUE         7
#define HUE_ERR_NOT_MODIFIABLE        8
        /* 9, 10 ? */
#define HUE_ERR_TOO_MANY             11
#define HUE_ERR_PORTAL_REQUIRED      12
#define HUE_ERR_INTERNAL_ERROR      901

/* Command Specific Error numbers and descriptions */
#define HUE_ERR_LINK_BUTTON_NOT_PUSHED          101
#define HUE_ERR_DHCP_NOT_DISABLED               110
#define HUE_ERR_INVALID_UPDATASTATE             111
#define HUE_ERR_PARAM_NOT_MODIFIABLE            201
#define HUE_ERR_COMMISSIONABLE_LIST_FULL        203
#define HUE_ERR_GROUP_TABLE_FULL                301
#define HUE_ERR_DELETE_NOT_PERMITTED            305
#define HUE_ERR_ALREADY_USED                    306
#define HUE_ERR_SCENE_BUFFER_FULL               402
#define HUE_ERR_SCENE_LOCKED                    403
#define HUE_ERR_GROUP_EMPTY                     404
#define HUE_ERR_CANNOT_CREATE_SENSOR            501
#define HUE_ERR_SENSOR_LIST_FULL                502
#define HUE_ERR_COMMISSIONABLE_SENSOR_LIST_FULL 503
#define HUE_ERR_RULE_ENGINE_FULL                601
#define HUE_ERR_CONDITION_ERROR                 607
#define HUE_ERR_ACTION_ERROR                    608
#define HUE_ERR_UNABLE_TO_ACTIVATE              609
#define HUE_ERR_SCHEDULE_LIST_FULL              701
#define HUE_ERR_INVALID_TIMEZONE                702
#define HUE_ERR_CANNOT_SET_SCHED_TIME           703
#define HUE_ERR_CANNOT_CREATE_SCHEDULE          704
#define HUE_ERR_SCHEDULE_IN_PAST                705
#define HUE_ERR_COMMAND_ERROR                   706
#define HUE_ERR_MODEL_INVALID                   801
#define HUE_ERR_FACTORY_NEW                     802
#define HUE_ERR_INVALID_STATE                   803


struct hue_entertainment_area
{
  uint16_t area_id;
  char area_name[AREA_NAME_LEN];
  uint16_t light_ids[MAX_LIGHTS_PER_AREA];
};


struct hue_whitelist_entry
{
  char *username;
  char *last_use_date;
  char *created_date;
  char *name;
};

struct hue_rest_ctx
{
  debug_cb_t debug_callback;
  int  debug_level;
  void *user_data;
  char *username;
  char *clientkey;
  char *address;
  int port;
  char *upload_data; /* Used for PUT/POST requests */
  size_t  upload_data_length;
  char *received_data;
  size_t  received_data_length;
  CURL *curl;
  struct hue_entertainment_area *ent_areas;
  struct hue_whitelist_entry *whitelist;
  uint whitelist_count;
  char devicetype[HUE_APP_NAME_SIZE + 1 + HUE_DEVICE_NAME_SIZE];
};


/* Function: hue_rest_init

   Initialise hue rest. Call once per app before calling any other functions. Be sure to call <hue_rest_cleanup> when finished with rest interface.

   Returns:

      0 on success, non-zero otherwise
*/
int hue_rest_init();

/* Function: hue_rest_cleanup

   Cleanup hue rest. Must be last hue rest function called (after the last call to <hue_rest_cleanup_ctx>)
*/
void hue_rest_cleanup();


/* Function: hue_rest_init_ctx

   Initialise hue rest context. Be sure to call <hue_rest_cleanup_ctx> when finished with the context.

   Parameters:

      ctx - Context to initialise
      debug_callback - Debug callback to receive debug message. If NULL, any/all debug output is sent to STDOUT
      address - IP address of hub
      port - Port of hub. Should probably always be 443
      username - App username: 40 character string generated by bridge when registering app
      debug_level - about of debugging output to generate. One of: MSG_OFF, MSG_ERR, MSG_INFO or MSG_DEBUG.

   Returns:

      0 on success, non-zero otherwise
*/
int hue_rest_init_ctx(struct hue_rest_ctx *ctx, debug_cb_t debug_callback, const char *address, int port, const char *username, int debug_level);

/* Function: hue_rest_cleanup_ctx

   Free any memory allocated when <hue_rest_init_ctx> was called.

   Parameters:

      ctx - hue_rest_ctx object
*/
void hue_rest_cleanup_ctx(struct hue_rest_ctx *ctx);

/* Function: hue_rest_activate_stream

   Instruct the brige to enable the streaming interface. Once enabled, a DTLS connection must be made
   to the bridge within 10 seconds, or streaming mode will be automatically disabled by the bridge.

   Parameters:

      ctx - hue_rest_ctx context
      group - Entertainment group ID to activate stream for. Use <hue_rest_get_ent_groups> to get a list of groups configured on the bridge.

   Returns:

      0 on success, non-zero otherwise
*/
int hue_rest_activate_stream(struct hue_rest_ctx *ctx, int group);

/* Function: hue_rest_get_ent_groups

   Get a list of entertainment groups configured on the bridge.

   Parameters:

      ctx - hue_rest_ctx context
      out_areas - pointer to pointer of hue_entertainment_area's, which includes group id, area name and a list of light IDs (up to 10). The memory pointed to by out_areas will be free'd either by the next call to <hue_rest_get_ent_groups>, or by <hue_rest_cleanup_ctx>.
      out_areas_count - Number of hue_entertainment_area in out_areas list.

   Returns:

      0 on success, non-zero otherwise
*/
int hue_rest_get_ent_groups(struct hue_rest_ctx *ctx, struct hue_entertainment_area **out_areas, int *out_areas_count);

/* Function: hue_rest_get_whitelist

   Get a list of apps registered on the bridge.

   Parameters:

      ctx - hue_rest_ctx context
      out_whitelist_entries - pointer to pointer of hue_whitelist_entry's, which includes username, date created, last used and name. The memory pointed to by out_whitelist_entries will be free'd either by the next call to <hue_rest_get_whitelist>, or by <hue_rest_cleanup_ctx>.
      out_whitelist_count - Number of hue_whitelist_entry in out_whitelist_entries list.

   Returns:

      0 on success, non-zero otherwise
*/
int hue_rest_get_whitelist(struct hue_rest_ctx *ctx, struct hue_whitelist_entry **out_whitelist_entries, uint *out_whitelist_count);

/* TODO */
int hue_rest_delete_user(struct hue_rest_ctx *ctx, const char *username);


/* Function: hue_rest_register

   Create a new user on the bridge. The link button the on the bridge must be pressed within the last 30 seconds before calling this for it to succeed.

   Parameters:

      ctx - hue_rest_ctx context
      out_username - on success, username generated by the bridge. Returned string is free'd by <hue_rest_cleanup_ctx>.
      out_clientkey - on success, PSK generated by the bridge which can be used for DTLS connections after <hue_rest_activate_stream> is called. Returned string is free'd by <hue_rest_cleanup_ctx>.

   Returns:

      - <0 on unknown error (likley unable to connect to bridge)
      -  0 on success, out_username & out_clientkey are populated
      - >0 error code returned by bridge. See HUE_ERR_* macros, but HUE_ERR_LINK_BUTTON_NOT_PUSHED is probably the most likley
*/
int hue_rest_register(struct hue_rest_ctx *ctx, char **out_username, char **out_clientkey);