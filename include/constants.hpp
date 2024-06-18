#pragma once

// #define LINKALHO_DEBUG
#define LOCAL_PATH "sdmc:/switch/" APP_TITLE_LOWER "/"
#define BACKUP_PATH LOCAL_PATH "backups/"
#define RESTORE_FILE_DIR LOCAL_PATH "restore/"
#define RESTORE_FILE_PATH RESTORE_FILE_DIR "restore.zip"

#ifdef LINKALHO_DEBUG
# define ACCOUNT_PATH LOCAL_PATH "debug"
#else
# define ACCOUNT_PATH "account:/su"
#endif

#define CUSTOM_PAYLOAD_FILE "reboot.bin"
#define CUSTOM_PAYLOAD_FILE_PATH LOCAL_PATH CUSTOM_PAYLOAD_FILE

#define DEFAULT_COUNTRY "Portugal"
#define DEFAULT_COUNTRY_CODE "PT"
#define DEFAULT_LOCALE "en-US"
#define DEFAULT_TIMEZONE "Europe/Lisbon"
