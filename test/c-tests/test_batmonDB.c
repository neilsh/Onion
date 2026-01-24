/**
 * Integration tests for batmonDB using Unity framework
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sqlite3/sqlite3.h>

#include "unity.h"

// Mock device serial
static char DEVICE_SN[13] = "TEST_DEV_001";

static bool is_file(const char *path) { return access(path, F_OK) == 0; }

#include "../../src/batmon/batmonDB.h"

void setUp(void) {
    if (bat_log_db) close_battery_log_db();
    remove(BATTERY_LOG_FILE);
}

void tearDown(void) {
    if (bat_log_db) close_battery_log_db();
    remove(BATTERY_LOG_FILE);
}

void test_db_open_creates_schema(void) {
    TEST_ASSERT_EQUAL(1, open_battery_log_db());
    TEST_ASSERT_NOT_NULL(bat_log_db);
    
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(bat_log_db, 
        "SELECT name FROM sqlite_master WHERE type='table' AND name='bat_activity';", 
        -1, &stmt, NULL);
    TEST_ASSERT_EQUAL(SQLITE_ROW, sqlite3_step(stmt));
    sqlite3_finalize(stmt);
    
    sqlite3_prepare_v2(bat_log_db,
        "SELECT name FROM sqlite_master WHERE type='table' AND name='device_specifics';",
        -1, &stmt, NULL);
    TEST_ASSERT_EQUAL(SQLITE_ROW, sqlite3_step(stmt));
    sqlite3_finalize(stmt);
}

void test_db_close_resets_pointer(void) {
    open_battery_log_db();
    close_battery_log_db();
    TEST_ASSERT_NULL(bat_log_db);
}

void test_best_session_defaults_to_zero(void) {
    TEST_ASSERT_EQUAL(0, get_best_session_time());
}

void test_best_session_retrieves_stored_value(void) {
    open_battery_log_db();
    char *sql = sqlite3_mprintf(
        "INSERT INTO device_specifics(device_serial, best_session) VALUES(%Q, 7200);", 
        DEVICE_SN);
    sqlite3_exec(bat_log_db, sql, NULL, NULL, NULL);
    sqlite3_free(sql);
    close_battery_log_db();
    
    TEST_ASSERT_EQUAL(7200, get_best_session_time());
}

void test_reopen_preserves_data(void) {
    open_battery_log_db();
    char *sql = sqlite3_mprintf(
        "INSERT INTO device_specifics(device_serial, best_session) VALUES(%Q, 3600);", 
        DEVICE_SN);
    sqlite3_exec(bat_log_db, sql, NULL, NULL, NULL);
    sqlite3_free(sql);
    close_battery_log_db();
    
    open_battery_log_db();
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(bat_log_db, 
        "SELECT best_session FROM device_specifics WHERE device_serial = ?;", 
        -1, &stmt, NULL);
    sqlite3_bind_text(stmt, 1, DEVICE_SN, -1, SQLITE_STATIC);
    TEST_ASSERT_EQUAL(SQLITE_ROW, sqlite3_step(stmt));
    TEST_ASSERT_EQUAL(3600, sqlite3_column_int(stmt, 0));
    sqlite3_finalize(stmt);
}

void test_bat_activity_insertion(void) {
    open_battery_log_db();
    char *sql = sqlite3_mprintf(
        "INSERT INTO bat_activity(device_serial, bat_level, duration, is_charging) "
        "VALUES(%Q, 85, 600, 0);", 
        DEVICE_SN);
    TEST_ASSERT_EQUAL(SQLITE_OK, sqlite3_exec(bat_log_db, sql, NULL, NULL, NULL));
    sqlite3_free(sql);
    
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(bat_log_db, 
        "SELECT bat_level, duration FROM bat_activity WHERE device_serial = ?;", 
        -1, &stmt, NULL);
    sqlite3_bind_text(stmt, 1, DEVICE_SN, -1, SQLITE_STATIC);
    TEST_ASSERT_EQUAL(SQLITE_ROW, sqlite3_step(stmt));
    TEST_ASSERT_EQUAL(85, sqlite3_column_int(stmt, 0));
    TEST_ASSERT_EQUAL(600, sqlite3_column_int(stmt, 1));
    sqlite3_finalize(stmt);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_db_open_creates_schema);
    RUN_TEST(test_db_close_resets_pointer);
    RUN_TEST(test_best_session_defaults_to_zero);
    RUN_TEST(test_best_session_retrieves_stored_value);
    RUN_TEST(test_reopen_preserves_data);
    RUN_TEST(test_bat_activity_insertion);
    return UNITY_END();
}
