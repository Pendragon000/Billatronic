idf_component_register(
        SRCS "Billatronic.c" "ism3.c" "mikrosdk_shim.c"
        INCLUDE_DIRS "."
        REQUIRES driver esp_driver_gpio esp_driver_spi)
