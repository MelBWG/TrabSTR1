set(srcs
    "HD44780.c")

set(includes "include")

idf_component_register(SRCS "${srcs}"
                       INCLUDE_DIRS ${includes}
                       REQUIRES driver)