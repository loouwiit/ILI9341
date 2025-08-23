idf_component_register(SRCS "main.cpp" "wifi.cpp"
                        "LCD/spi.cpp" "LCD/color.cpp" "LCD/ILI9341.cpp" "LCD/iic.cpp" "LCD/FT6X36.cpp" "LCD/clickable.cpp"
                        "app/clock/clock.cpp"
                        "app/desktop/desktop.cpp"
                        "app/setting/setting.cpp" "app/setting/wifiSetting.cpp" "app/setting/timeSetting.cpp" "app/setting/systemInfo.cpp" "app/setting/touchTest.cpp"
                        "app/input/input.cpp"
                    INCLUDE_DIRS "LCD" "app" "."
                    PRIV_REQUIRES esp_driver_gpio esp_driver_spi esp_driver_i2c nvs_flash esp_wifi spi_flash)
