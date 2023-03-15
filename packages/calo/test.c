#include <stdio.h>
#include <ek/buf.h>
#include <calo.h>
#include <gen_sg.h>

int main() {

    sg_file_t file = {0};
    sg_node_data_t node = {0};
    sg_node_data_t node_ch = {0};
    sg_node_data_t node_ch2 = {0};
    sg_node_data_t node_ch3 = {0};
    arr_push(node.children, node_ch);
    arr_push(node_ch.children, node_ch2);
    arr_push(node_ch2.children, node_ch3);
    arr_push(node.children, node_ch);
    arr_push(node.children, node_ch);
    arr_push(file.library, node);
    arr_push(file.library, node);
    arr_push(file.library, node);
    arr_push(file.library, node);
    sg_scene_info_t linkage = {0};
    arr_push(file.linkages, linkage);
    arr_push(file.linkages, linkage);
    arr_push(file.linkages, linkage);
    arr_push(file.linkages, linkage);
    uint32_t s = 0x22;
    arr_push(file.scenes, s++);
    arr_push(file.scenes, s++);
    arr_push(file.scenes, s++);
    arr_push(file.scenes, s++);

    calo_writer_t writer = new_writer(1);
    write_stream_sg_file(&writer, file);

    FILE* f = fopen("test.sg", "wb");
    fwrite(writer.data, 1, writer.p - (uint8_t*)writer.data, f);
    fclose(f);

    uint32_t size;
    uint8_t* buf = read_file("test.sg", &size);
    calo_reader_t reader = {0};
    reader.p = buf;
    sg_file_t file2 = read_stream_sg_file(&reader);
    printf("%u %u %u %u \n",
           size,
           arr_size(file2.library),
           arr_size(file2.linkages),
           arr_size(file2.scenes)
           );

    return 0;
}


int test_tables() {
    uint32_t indices[1024];
    uint32_t indices_num = 0;
    {
        vla_table_t tab = {0};
        indices[indices_num++] = add_string(&tab, "");
        indices[indices_num++] = add_string(&tab, "hello");
        indices[indices_num++] = add_string(&tab, "string2");
        indices[indices_num++] = add_string(&tab, "duplicated strings");
        indices[indices_num++] = add_string(&tab, "");
        indices[indices_num++] = add_string(&tab, "0\x00");
        indices[indices_num++] = add_string(&tab, "VERY VERY VERY VERY LONG STRING!");
        indices[indices_num++] = add_string(&tab, "duplicated strings");
        indices[indices_num++] = add_string(&tab, "duplicated strings");
        indices[indices_num++] = add_string(&tab, "duplicated strings");
        indices[indices_num++] = add_string(&tab, "duplicated strings");
        indices[indices_num++] = add_string(&tab, "duplicated strings");
        indices[indices_num++] = add_string(&tab, "duplicated strings");
        indices[indices_num++] = add_string(&tab, "duplicated strings");
        indices[indices_num++] = add_string(&tab, "{end of strings section}");

        FILE* f = fopen("test.alo", "wb");
        fwrite_table(f, &tab);
        fflush(f);
        fclose(f);
        free_table(&tab);
    }
    {
        void* mem = read_file("test.alo", 0);
        calo_reader_t reader = {0};
        map_table(&reader, mem);
        vla_table_t * t = &reader.strings;

        printf("data section:\n");
        for (uint32_t i = 0; i < t->count; ++i) {
            printf("%u: %s\n", i, get_data(t, i));
        }

        printf("\n restore order and strings:\n");
        for (uint32_t i = 0; i < indices_num; ++i) {
            printf("\"%s\"\n", get_data(t, indices[i]));
        }

        // don't delete table, because it's mapped
        // free_table(&t);

        // free file content instead
        free(mem);
    }
    return 0;
}

