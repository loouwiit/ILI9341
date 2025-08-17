idf_component_register(SRCS "iic.cpp" "spi.cpp" "iic.cpp" "main.cpp" "LCD/color.cpp" "LCD/ILI9341.cpp" "LCD/FT6X36.cpp"
                    INCLUDE_DIRS "LCD" "."
                    PRIV_REQUIRES esp_driver_gpio esp_driver_spi esp_driver_i2c)
