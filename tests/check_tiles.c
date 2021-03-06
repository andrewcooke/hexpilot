
#include <check.h>
#include <stdlib.h>
#include <float.h>
#include <math.h>

#include "lu/log.h"

#include "../src/tiles.h"


START_TEST(test_small_hexagon) {
    lulog *log;
    luary_fxyzw *vertices = NULL;
    luary_uint32 *indices = NULL, *counts = NULL;
    luary_void *offsets = NULL;
    ck_assert(!lulog_stderr_mk(&log, lulog_level_debug));
    ck_assert(!hexagon_vertex_strips(log, 0, 1, 1, 1.0, 1.0, &vertices, &indices, &offsets, &counts));
    luary_fxyzw_dump(log, vertices, "vertices", 10);
    ck_assert_msg(vertices->mem.used == 7, "Expected 7 vertices, found %zu", vertices->mem.used);
    ck_assert(vertices->fxyzw[2].x == -1);
    ck_assert(vertices->fxyzw[2].y == 0);
    ck_assert(vertices->fxyzw[2].z == 0);
    ck_assert(vertices->fxyzw[2].w == 1);
    luary_void_dump(log, offsets, "offsets", 10);
    ck_assert_msg(offsets->mem.used == 2, "Expected 2 offsets, found %zu", offsets->mem.used);
    ck_assert(offsets->ptr[0] == (void*)(0*4));
    ck_assert(offsets->ptr[1] == (void*)(6*4));
    luary_uint32_dump(log, indices, "indices", 10);
    ck_assert_msg(indices->mem.used == 11, "Expected 11 indices, found %zu", indices->mem.used);
    ck_assert_msg(indices->i[0] == 2, "index 0 == %d", indices->i[0]);
    ck_assert_msg(indices->i[1] == 2, "index 1 == %d", indices->i[0]);
    ck_assert_msg(indices->i[2] == 0, "index 2 == %d", indices->i[1]);
    ck_assert_msg(indices->i[3] == 3, "index 3 == %d", indices->i[2]);
    ck_assert_msg(indices->i[4] == 1, "index 4 == %d", indices->i[2]);
    ck_assert_msg(indices->i[5] == 4, "index 5 == %d", indices->i[2]);
    ck_assert_msg(indices->i[6] == 2, "index 6 == %d", indices->i[2]);
    ck_assert_msg(indices->i[7] == 5, "index 7 == %d", indices->i[2]);
    ck_assert_msg(indices->i[8] == 3, "index 8 == %d", indices->i[2]);
    ck_assert_msg(indices->i[9] == 6, "index 9 == %d", indices->i[2]);
    ck_assert_msg(indices->i[10] == 4, "index 10 == %d", indices->i[2]);
    luary_uint32_dump(log, counts, "counts", 10);
    ck_assert_msg(counts->mem.used == 2, "Expected 2 counts, found %zu", counts->mem.used);
    ck_assert(counts->i[0] == 6);
    ck_assert(counts->i[1] == 5);
    ck_assert(!luary_uint32_free(&indices, 0));
    ck_assert(!luary_void_free(&offsets, 0));
    ck_assert(!luary_uint32_free(&counts, 0));
    ck_assert(!luary_fxyzw_free(&vertices, 0));
    ck_assert(!log->free(&log, 0));
} END_TEST


START_TEST(test_norms) {
    lulog *log;
    luary_vnorm *vertices = NULL;
    luary_uint32 *counts = NULL;
    luary_int32 *offsets = NULL;
    ck_assert(!lulog_stderr_mk(&log, lulog_level_debug));
    ck_assert(!hexagon_vnormal_strips(log, 0, 1, 1, 1.0, 1.0, &vertices, &offsets, &counts));
    luary_vnorm_dump(log, vertices, "vertices", 10);
    ck_assert_msg(vertices->mem.used == 11, "Expected 11 vertices, found %zu", vertices->mem.used);
    luary_int32_dump(log, offsets, "offsets", 10);
    ck_assert_msg(offsets->mem.used == 2, "Expected 2 offsets, found %zu", offsets->mem.used);
    ck_assert(offsets->i[0] == 0);
    ck_assert(offsets->i[1] == 6);
    luary_uint32_dump(log, counts, "counts", 10);
    ck_assert_msg(counts->mem.used == 2, "Expected 2 counts, found %zu", counts->mem.used);
    ck_assert(counts->i[0] == 6);
    ck_assert(counts->i[1] == 5);
    ck_assert(!luary_int32_free(&offsets, 0));
    ck_assert(!luary_uint32_free(&counts, 0));
    ck_assert(!luary_vnorm_free(&vertices, 0));
    ck_assert(!log->free(&log, 0));
} END_TEST


int main(void) {

    int failed = 0;
    Suite *s;
    TCase *c;
    SRunner *r;

    c = tcase_create("case");
    tcase_add_test(c, test_small_hexagon);
    tcase_add_test(c, test_norms);
    s = suite_create("suite");
    suite_add_tcase(s, c);
    r = srunner_create(s);
    srunner_run_all(r, CK_VERBOSE);
    failed = srunner_ntests_failed(r);
    srunner_free(r);

    return failed ? 1 : 0;
}
