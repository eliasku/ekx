#include <unit.h>
#include <ek/sigslot.h>
#include <ek/buf.h>

static void cb_test(signal_slot_t* slot, void* userdata) {
    int* pi = (int*)slot->context.ptr;
    *pi += *((int*)userdata);
}

SUITE(sigslot) {
    IT("basic") {
        signal_t signal = {0};

        int r = 0;
        signal_slot_t* slot1 = sig_connect(&signal, cb_test, 0);
        slot1->context.ptr = &r;

        signal_slot_t* slot2 = sig_connect(&signal, cb_test, 1);
        slot2->context.ptr = &r;
        slot2->once = true;

        int add1 = 1;
        int add2 = 3;

        // +3 (only slot1)
        sig_emit(&signal, 2, &add2);
        REQUIRE_EQ(r, 3);
        // +3, +3, remove slot2 (once)
        sig_emit(&signal, 1, &add2);
        REQUIRE_EQ(r, 9);
        // +1
        sig_emit(&signal, 0, &add1);
        REQUIRE_EQ(r, 10);
        // +3
        sig_emit(&signal, 0, &add2);
        REQUIRE_EQ(r, 13);

        sig_close(&signal);
        REQUIRE_EQ(arr_size(signal.slots), 0);
    }
}