idf_component_register(SRCS "ILI9341.cpp" "spi.cpp" "main.cpp"
                    INCLUDE_DIRS ""
                    PRIV_REQUIRES esp_driver_gpio esp_driver_spi)
