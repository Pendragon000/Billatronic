idf_component_register(
        SRCS "Billatronic.c"
        INCLUDE_DIRS "."
        REQUIRES driver esp_driver_gpio led Keypad)
