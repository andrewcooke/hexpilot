
#include <check.h>
#include <stdlib.h>
#include <float.h>
#include <math.h>

#include "lu/log.h"

#include "../src/tiles.h"
#include "../src/vertices.h"


START_TEST(test_small_hexagon) {
    lulog *log;
    luarray_fxyzw *vertices = NULL;
    luarray_uint32 *indices = NULL, *counts = NULL;
    luarray_void *offsets = NULL;
    ck_assert(!lulog_mkstderr(&log, lulog_level_debug));
    ck_assert(!hexagon_vertex_strips(log, 0, 1, 1, 1.0, 1.0, &vertices, &indices, &offsets, &counts));
    luarray_dumpfxyzw(log, vertices, "vertices", 10);
    ck_assert_msg(vertices->mem.used == 7, "Expected 7 vertices, found %zu", vertices->mem.used);
    ck_assert(vertices->fxyzw[2].x == -1);
    ck_assert(vertices->fxyzw[2].y == 0);
    ck_assert(vertices->fxyzw[2].z == 0);
    ck_assert(vertices->fxyzw[2].w == 1);
    luarray_dumpvoid(log, offsets, "offsets", 10);
    ck_assert_msg(offsets->mem.used == 2, "Expected 2 offsets, found %zu", offsets->mem.used);
    ck_assert(offsets->ptr[0] == (void*)(0*4));
    ck_assert(offsets->ptr[1] == (void*)(5*4));
    luarray_dumpuint32(log, indices, "indices", 10);
    ck_assert_msg(indices->mem.used == 10, "Expected 10 indices, found %zu", indices->mem.used);
    ck_assert_msg(indices->i[0] == 2, "index 0 == %d", indices->i[0]);
    ck_assert_msg(indices->i[1] == 0, "index 1 == %d", indices->i[1]);
    ck_assert_msg(indices->i[2] == 3, "index 2 == %d", indices->i[2]);
    ck_assert_msg(indices->i[3] == 1, "index 3 == %d", indices->i[2]);
    ck_assert_msg(indices->i[4] == 4, "index 4 == %d", indices->i[2]);
    ck_assert_msg(indices->i[5] == 2, "index 5 == %d", indices->i[2]);
    ck_assert_msg(indices->i[6] == 5, "index 6 == %d", indices->i[2]);
    ck_assert_msg(indices->i[7] == 3, "index 7 == %d", indices->i[2]);
    ck_assert_msg(indices->i[8] == 6, "index 8 == %d", indices->i[2]);
    ck_assert_msg(indices->i[9] == 4, "index 9 == %d", indices->i[2]);
    luarray_dumpuint32(log, counts, "counts", 10);
    ck_assert_msg(counts->mem.used == 2, "Expected 2 counts, found %zu", counts->mem.used);
    ck_assert(counts->i[0] == 5);
    ck_assert(counts->i[1] == 5);
    ck_assert(!luarray_freeuint32(&indices, 0));
    ck_assert(!luarray_freevoid(&offsets, 0));
    ck_assert(!luarray_freeuint32(&counts, 0));
    ck_assert(!luarray_freefxyzw(&vertices, 0));
    ck_assert(!log->free(&log, 0));
} END_TEST


START_TEST(test_norms) {
    lulog *log;
    luarray_vnorm *vertices = NULL;
    luarray_uint32 *counts = NULL;
    luarray_int32 *offsets = NULL;
    ck_assert(!lulog_mkstderr(&log, lulog_level_debug));
    ck_assert(!hexagon_vnormal_strips(log, 0, 1, 1, 1.0, 1.0, &vertices, &offsets, &counts));
    luarray_dumpvnorm(log, vertices, "vertices", 10);
    ck_assert_msg(vertices->mem.used == 10, "Expected 10 vertices, found %zu", vertices->mem.used);
//    ck_assert(vertices->v[2].v.x == -1);
//    ck_assert(vertices->v[2].v.y == 0);
//    ck_assert(vertices->v[2].v.z == 0);
//    ck_assert(vertices->v[2].v.w == 1);
    luarray_dumpint32(log, offsets, "offsets", 10);
    ck_assert_msg(offsets->mem.used == 2, "Expected 2 offsets, found %zu", offsets->mem.used);
    ck_assert(offsets->i[0] == 0);
    ck_assert(offsets->i[1] == 5);
    luarray_dumpuint32(log, counts, "counts", 10);
    ck_assert_msg(counts->mem.used == 2, "Expected 2 counts, found %zu", counts->mem.used);
    ck_assert(counts->i[0] == 5);
    ck_assert(counts->i[1] == 5);
    ck_assert(!luarray_freeint32(&offsets, 0));
    ck_assert(!luarray_freeuint32(&counts, 0));
    ck_assert(!luarray_freevnorm(&vertices, 0));
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
