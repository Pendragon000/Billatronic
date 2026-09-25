idf_component_register(SRCS "ism3.c" "mikrosdk_shim.c"
        INCLUDE_DIRS "include"
        REQUIRES esp_driver_gpio esp_driver_spi)