idf_component_register(SRCS "spi.cpp" "main.cpp" "LCD/color.cpp" "LCD/ILI9341.cpp"
                    INCLUDE_DIRS "LCD" "."
                    PRIV_REQUIRES esp_driver_gpio esp_driver_spi)
