import {readFileSync, writeFileSync} from "fs";
import {get_writer_bytes, newReader, newWriter, read_table, write_raw_bytes, write_table} from "./calo.js";
import {read_stream_sg_file, write_stream_sg_file, write_stream_sg_keyframe_transform} from "./gen_sg.js";

const filepath = "/Users/ilyak/ek/all/ekx-demo/build/assets/tests.sg";
const filepath2 = "/Users/ilyak/ek/all/ekx-demo/build/assets/tests_.sg";
const buf = readFileSync(filepath);
const reader = newReader(buf);
read_table(reader);
const scenes = read_stream_sg_file(reader);

console.info(reader._strings.strings.join("\n"));
console.info(scenes.library);


const writer = newWriter(1);
const writer2 = newWriter(1);
write_stream_sg_file(writer, scenes);
write_table(writer2, writer._strings);
write_raw_bytes(writer2, get_writer_bytes(writer));
writeFileSync(filepath2, get_writer_bytes(writer2));