/*
    user_gcode_example.cpp 

    Copyright (c) 2021    Barton Dring @buildlog

    Used to add simple G or M codes to the firmware
    If in doubt use an M code 

    You can pass letter values like 

    see the definiitons of parser_block_t and gc_values_t
    Note: letters have different types like unit32_t and float

*/

char    current_letter   = '\0';
uint8_t current_code_num = 0;

/*
    Used as a basic validator to tell Grbl_ESP32 that the gcode is supported in this file.
    Do not execute the gcode. That is in another function

        letter          'G' or 'M' ('M' in the case of M115)
        code_num        xxx (115 in the case of M115)  
        value_words     This will be sent as zer. You need to add every one you need (M114 E15 would need bit(GCodeWord::E) added to it)

        return          return true if the letter and code_num are support

*/
bool user_validate_gcode(char letter, uint8_t code_num, uint32_t& value_words) {
    current_letter   = letter;
    current_code_num = code_num;

    if (current_letter == 'G') {
        // we don't support anythibg yet
    } else if (current_letter == 'M') {
        switch (current_code_num) {
            case 115:
                //bit_true(value_words, bit(GCodeWord::E)); (not used, just here an an example)
                return true;
            default:
                break;
        }
    }

    current_letter   = '\0';
    current_code_num = 0;

    return false;
}

/*
    Execute the gcode. 

    parse_block contains a lot of info, most notably the letter and values like P12
*/

bool user_execute_gcode(parser_block_t parser_block) {
    if (current_letter == 'G') {
        return false;
    } else if (current_letter == 'M') {
        switch (current_code_num) {
            case 115:
                //grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "Execute %c%d %d", current_letter, current_code_num, parser_block.values.e);
                grbl_sendf(
                    CLIENT_SERIAL, "FIRMWARE_NAME:Grbl_ESP32, FIRMWARE_VERSION:%s, FIRMWARE_BUILD:%s\r\n", GRBL_VERSION, GRBL_VERSION_BUILD);
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