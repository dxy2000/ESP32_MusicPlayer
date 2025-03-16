set(SOURCES "main.cc" 
            "ssd1306Display.cpp"
            "font/myIcon.c"
            "font/hanyi_14.c")

set(INCLUDE_DIRS ".")

idf_component_register(SRCS ${SOURCES}
                    INCLUDE_DIRS ${INCLUDE_DIRS})