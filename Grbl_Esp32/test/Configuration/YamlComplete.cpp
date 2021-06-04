#include "../TestFramework.h"

#include <src/Configuration/Tokenizer.h>
#include <src/Configuration/Parser.h>

namespace Configuration {
    Test(YamlComplete, Test) {
        const char* config = "name: \"ESP32 Dev Controller V4\"\n"
                             "board: \"ESP32 Dev Controller V4\"\n"
                             "yaml_wiki: \"https://github.com/bdring/Grbl_Esp32/wiki/YAML-Config-File\"\n"
                             "\n"
                             "idle_time: 250\n"
                             "step_type: rmt\n"
                             "dir_delay_microseconds: 1\n"
                             "pulse_microseconds: 2\n"
                             "disable_delay_us: 0\n"
                             "homing_init_lock: false\n"
                             "\n"
                             "axes:\n"
                             "  number_axis: 3\n"
                             "  shared_stepper_disable: gpio.13:low\n"
                             "  \n"
                             "  x:\n"
                             "    steps_per_mm: 800\n"
                             "    max_rate: 2000\n"
                             "    acceleration: 25\n"
                             "    max_travel: 1000\n"
                             "    home_mpos: 10\n"
                             "    \n"
                             "    gang0:\n"
                             "      endstops:\n"
                             "        dual: gpio.17:low:pu\n"
                             "      stepstick:\n"
                             "        direction: gpio:14\n"
                             "        step: gpio.12\n"
                             "    gang1:\n"
                             "      null_motor:\n"
                             "\n"
                             "  y:\n"
                             "    steps_per_mm: 800\n"
                             "    max_rate: 2000\n"
                             "    acceleration: 25\n"
                             "    max_travel: 1000\n"
                             "    home_mpos: 10\n"
                             "\n"
                             "    gang0:\n"
                             "      endstops:\n"
                             "        dual: gpio.4:high:pd\n"
                             "      stepstick:\n"
                             "        direction: gpio:15\n"
                             "        step: gpio.26\n"
                             "    gang1:\n"
                             "      null_motor:\n"
                             "\n"
                             "  z:\n"
                             "    steps_per_mm: 800\n"
                             "    max_rate: 2000\n"
                             "    acceleration: 25\n"
                             "    max_travel: 1000\n"
                             "    home_mpos: 10\n"
                             "\n"
                             "    gang0:\n"
                             "      endstops:\n"
                             "        dual: gpio.16:pu:low\n"
                             "      stepstick:\n"
                             "        direction: gpio:33\n"
                             "        step: gpio.27\n"
                             "    gang1:\n"
                             "      null_motor:\n"
                             "\n"
                             "\n"
                             "coolant:\n"
                             "  flood: gpio.25:low\n"
                             "  mist:  gpio.21\n"
                             "\n"
                             "comms:\n"
                             "    wifi_sta:\n"
                             "        ssid: StefanMieke\n"
                             "\n"
                             "    wifi_ap:\n"
                             "        ip_address: \"192.168.0.1\"\n"
                             "        ssid: ScratchThat\n"
                             "        \n"
                             "probe:\n"
                             "    pin: gpio.32:high:pu\n"
                             "\n";

        Parser p(config, config + strlen(config));
        {
            Assert(!p.isEndSection(), "No EOS expected");
            Assert(p.key().equals("name"), "Expected 'name'");
            Assert(p.moveNext(), "Move next failed.");

            Assert(!p.isEndSection(), "No EOS expected");
            Assert(p.key().equals("board"), "Expected 'board'");
            Assert(p.moveNext(), "Move next failed.");

            Assert(!p.isEndSection(), "No EOS expected");
            Assert(p.key().equals("yaml_wiki"), "Expected 'yaml_wiki'");
            Assert(p.moveNext(), "Move next failed.");

            Assert(!p.isEndSection(), "No EOS expected");
            Assert(p.key().equals("idle_time"), "Expected 'idle_time'");
            Assert(p.moveNext(), "Move next failed.");

            Assert(!p.isEndSection(), "No EOS expected");
            Assert(p.key().equals("step_type"), "Expected 'step_type'");
            Assert(p.moveNext(), "Move next failed.");

            Assert(!p.isEndSection(), "No EOS expected");
            Assert(p.key().equals("dir_delay_microseconds"), "Expected 'dir_delay_microseconds'");
            Assert(p.moveNext(), "Move next failed.");

            Assert(!p.isEndSection(), "No EOS expected");
            Assert(p.key().equals("pulse_microseconds"), "Expected 'pulse_microseconds'");
            Assert(p.moveNext(), "Move next failed.");

            Assert(!p.isEndSection(), "No EOS expected");
            Assert(p.key().equals("disable_delay_us"), "Expected 'disable_delay_us'");
            Assert(p.moveNext(), "Move next failed.");

            Assert(!p.isEndSection(), "No EOS expected");
            Assert(p.key().equals("homing_init_lock"), "Expected 'homing_init_lock'");
            Assert(p.moveNext(), "Move next failed.");

            Assert(!p.isEndSection(), "No EOS expected");
            Assert(p.key().equals("axes"), "Expected 'axes'");
            {
                p.enter();

                Assert(!p.isEndSection(), "No EOS expected");
                Assert(p.key().equals("number_axis"), "Expected 'number_axis'");
                Assert(p.moveNext(), "Move next failed.");
                Assert(!p.isEndSection(), "No EOS expected");
                Assert(p.key().equals("shared_stepper_disable"), "Expected 'shared_stepper_disable'");
                Assert(p.moveNext(), "Move next failed.");
                Assert(!p.isEndSection(), "No EOS expected");
                Assert(p.key().equals("x"), "Expected 'x'");
                Assert(p.moveNext(), "Move next failed.");
                Assert(!p.isEndSection(), "No EOS expected");
                Assert(p.key().equals("y"), "Expected 'y'");
                Assert(p.moveNext(), "Move next failed.");
                Assert(!p.isEndSection(), "No EOS expected");
                Assert(p.key().equals("z"), "Expected 'z'");
                Assert(!p.moveNext(), "Move next failed.");
                Assert(p.isEndSection(), "EOS expected");

                p.leave();
            }
            Assert(!p.isEndSection(), "No EOS expected");
            Assert(p.moveNext(), "Move next failed.");

            Assert(p.key().equals("coolant"), "Expected 'coolant'");
            {
                p.enter();

                Assert(!p.isEndSection(), "No EOS expected");
                Assert(p.key().equals("flood"), "Expected 'flood'");
                Assert(p.moveNext(), "Move next failed.");
                Assert(!p.isEndSection(), "No EOS expected");
                Assert(p.key().equals("mist"), "Expected 'mist'");
                Assert(!p.moveNext(), "Move next failed.");
                Assert(p.isEndSection(), "No EOS expected");

                p.leave();
            }
            Assert(!p.isEndSection(), "No EOS expected");
            Assert(p.moveNext(), "Move next failed.");

            Assert(p.key().equals("comms"), "Expected 'comms'");
            {
                p.enter();

                Assert(!p.isEndSection(), "No EOS expected");
                Assert(p.key().equals("wifi_sta"), "Expected 'wifi_sta'");
                {
                    p.enter();
                    Assert(!p.isEndSection(), "No EOS expected");
                    Assert(p.key().equals("ssid"), "Expected 'ssid'");
                    Assert(!p.moveNext(), "Move next failed.");
                    Assert(p.isEndSection(), "No EOS expected");
                    p.leave();
                }
                Assert(!p.isEndSection(), "No EOS expected");
                Assert(p.moveNext(), "Move next failed.");
                Assert(p.key().equals("wifi_ap"), "Expected 'wifi_ap'");
                Assert(!p.moveNext(), "Move next failed.");
                Assert(p.isEndSection(), "EOS expected");

                p.leave();
            }
            Assert(!p.isEndSection(), "No EOS expected");
            Assert(p.moveNext(), "Move next failed.");

            Assert(p.key().equals("probe"), "Expected 'probe'");
            Assert(!p.moveNext(), "Move next failed.");
            Assert(p.isEndSection(), "EOS expected");
        }
    }
}
