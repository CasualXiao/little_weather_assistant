#Add sources from ui directory
file(GLOB_RECURSE SRC_UI ${CMAKE_SOURCE_DIR} "ui/*.c")

idf_component_register(SRCS 
                    "main.c"
                    "hardware/gxhtc3/bsp_gxhtc3.c"
                    "hardware/timer/bsp_timer.c"
                    "hardware/hf_weather/hf_weather.c"
                    "hardware/lcd/bsp_lcd.c"
                    "hardware/net/bsp_net.c"
                    "ui_function/ui_func.c"
                    ${SRC_UI}

                    INCLUDE_DIRS 
                    "."
                    "./ui_function"
                    "./hardware/lcd"
                    "./hardware/net"
                    "./hardware/gxhtc3"
                    "./hardware/timer"
                    "./hardware/hf_weather"
                    "ui"
                    )
