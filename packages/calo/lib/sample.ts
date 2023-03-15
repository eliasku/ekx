import {array, CString, f32, type, u32} from "./common.js";

const Player = type("Player", {
    name: CString,
    hp: u32,
    hp_max: u32,
    value: f32,
});

const file = {
    players: array(Player),
};


