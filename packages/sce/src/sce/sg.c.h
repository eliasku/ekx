#include <sce/sg.h>

struct res_sg res_sg;

void setup_res_sg(void) {
    struct res_sg* R = &res_sg;
    rr_man_t* rr = &R->rr;

    rr->names = R->names;
    rr->data = R->data;
    rr->max = sizeof(R->data) / sizeof(R->data[0]);
    rr->num = 1;
    rr->data_size = sizeof(R->data[0]);
}

void sg_load(sg_file_t* out, const void* data, uint32_t size) {
    if (size > 0) {
        calo_reader_t reader = {0};
        reader.p = (uint8_t*) data;
        read_calo(&reader);
        *out = read_stream_sg_file(&reader);
    } else {
        log_error("SCENE LOAD: empty buffer");
    }
}

const sg_node_data_t* sg_get(const sg_file_t* sg, string_hash_t library_name) {
    // TODO: optimize access!
    uint32_t count = arr_size(sg->library);
    for (uint32_t i = 0; i < count; ++i) {
        sg_node_data_t* item = sg->library + i;
        if (item->library_name == library_name) {
            return item;
        }
    }
    return NULL;
}
