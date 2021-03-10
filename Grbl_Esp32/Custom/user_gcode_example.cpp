

bool user_validate_gcode(char letter, uint8_t code_num, uint32_t &value_words) {
    grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "Testing gcode");

    if (letter == 'G') {
        return false;
    } else if (letter == 'M') {
        switch (code_num) {
            case 115:
                bit_false(value_words, bit(GCodeWord::E));
               
                return true;
                break;
            default:
                return false;
                break;
        }
        
    } else {
        return false;  // default is to reject all unknown numbers
    }
}

bool user_execute_gcode(char letter, uint8_t code_num, parser_block_t parser_block) {
    if (letter == 'G') {
        return false;
    } else if (letter == 'M') {
        switch (code_num) {
            case 115:
                grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "Execute M115 %d", parser_block.values.e);

                return true;
                break;
            default:
                return false;
                break;
        }

    } else {
        return false;  // default is to reject all unknown numbers
    }
}