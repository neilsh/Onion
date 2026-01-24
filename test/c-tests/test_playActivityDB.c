/**
 * Integration tests for playActivityDB using Unity framework
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/stat.h>

#include "unity.h"

// Stub cache functions before including playActivityDB.h
#define CACHE_DB_H
typedef struct { char *cache_path, *name, *path, *imgpath; } CacheDBItem;
static CacheDBItem* cache_db_find(const char *p) { (void)p; return NULL; }
#define CACHE_NOT_FOUND -1
static int cache_get_path(char *a, char *b, const char *c) { (void)a;(void)b;(void)c; return -1; }

#include "../../src/playActivity/playActivityDB.h"

// Unity setUp/tearDown - called automatically before/after each test
void setUp(void) {
    if (play_activity_db) play_activity_db_close();
    remove(PLAY_ACTIVITY_DB_NEW_FILE);
    mkdir(ROMS_FOLDER, 0777);
}

void tearDown(void) {
    if (play_activity_db) play_activity_db_close();
    remove(PLAY_ACTIVITY_DB_NEW_FILE);
}

// Tests
void test_db_open_creates_schema(void) {
    play_activity_db_open();
    TEST_ASSERT_NOT_NULL(play_activity_db);
    
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(play_activity_db, 
        "SELECT name FROM sqlite_master WHERE type='table' AND name='rom';", 
        -1, &stmt, NULL);
    TEST_ASSERT_EQUAL(SQLITE_ROW, sqlite3_step(stmt));
    sqlite3_finalize(stmt);
    
    sqlite3_prepare_v2(play_activity_db,
        "SELECT name FROM sqlite_master WHERE type='table' AND name='play_activity';",
        -1, &stmt, NULL);
    TEST_ASSERT_EQUAL(SQLITE_ROW, sqlite3_step(stmt));
    sqlite3_finalize(stmt);
}

void test_db_open_is_idempotent(void) {
    play_activity_db_open();
    sqlite3 *first = play_activity_db;
    play_activity_db_open();
    TEST_ASSERT_EQUAL_PTR(first, play_activity_db);
}

void test_db_close_resets_pointer(void) {
    play_activity_db_open();
    play_activity_db_close();
    TEST_ASSERT_NULL(play_activity_db);
}

void test_insert_and_find_rom(void) {
    play_activity_db_open();
    int id = __db_insert_rom("NES", "Mario", "NES/mario.nes", "");
    TEST_ASSERT_GREATER_THAN(0, id);
    TEST_ASSERT_EQUAL(id, __db_get_rom_id_by_path("NES/mario.nes"));
}

void test_rom_not_found_returns_sentinel(void) {
    play_activity_db_open();
    TEST_ASSERT_EQUAL(ROM_NOT_FOUND, __db_get_rom_id_by_path("nonexistent.rom"));
}

void test_record_play_session(void) {
    play_activity_db_open();
    int id = __db_insert_rom("NES", "Test", "NES/test.nes", "");
    char *sql = sqlite3_mprintf("INSERT INTO play_activity(rom_id, play_time) VALUES(%d, 3600);", id);
    sqlite3_exec(play_activity_db, sql, NULL, NULL, NULL);
    sqlite3_free(sql);
    play_activity_db_close();
    
    TEST_ASSERT_EQUAL(3600, play_activity_get_play_time("NES/test.nes"));
}

void test_total_play_time_filters_short_sessions(void) {
    play_activity_db_open();
    int r1 = __db_insert_rom("A", "Short", "A/s.nes", "");
    int r2 = __db_insert_rom("B", "Long", "B/l.nes", "");
    char *s1 = sqlite3_mprintf("INSERT INTO play_activity(rom_id, play_time) VALUES(%d, 30);", r1);
    char *s2 = sqlite3_mprintf("INSERT INTO play_activity(rom_id, play_time) VALUES(%d, 120);", r2);
    sqlite3_exec(play_activity_db, s1, NULL, NULL, NULL);
    sqlite3_exec(play_activity_db, s2, NULL, NULL, NULL);
    sqlite3_free(s1); sqlite3_free(s2);
    play_activity_db_close();
    
    TEST_ASSERT_EQUAL_MESSAGE(120, play_activity_get_total_play_time(), 
        "Only sessions > 60s should be counted");
}

void test_reopen_preserves_data(void) {
    play_activity_db_open();
    int id = __db_insert_rom("GBA", "Pokemon", "GBA/pokemon.gba", "");
    char *sql = sqlite3_mprintf("INSERT INTO play_activity(rom_id, play_time) VALUES(%d, 7200);", id);
    sqlite3_exec(play_activity_db, sql, NULL, NULL, NULL);
    sqlite3_free(sql);
    play_activity_db_close();
    
    play_activity_db_open();
    TEST_ASSERT_EQUAL(id, __db_get_rom_id_by_path("GBA/pokemon.gba"));
    play_activity_db_close();
    
    TEST_ASSERT_EQUAL(7200, play_activity_get_play_time("GBA/pokemon.gba"));
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_db_open_creates_schema);
    RUN_TEST(test_db_open_is_idempotent);
    RUN_TEST(test_db_close_resets_pointer);
    RUN_TEST(test_insert_and_find_rom);
    RUN_TEST(test_rom_not_found_returns_sentinel);
    RUN_TEST(test_record_play_session);
    RUN_TEST(test_total_play_time_filters_short_sessions);
    RUN_TEST(test_reopen_preserves_data);
    return UNITY_END();
}
