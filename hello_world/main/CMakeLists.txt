idf_component_register(SRCS "lcd_driver.c" "luz_driver.c" "temp_driver.c" "main.c" "server_motor.c"
                    INCLUDE_DIRS "."
                                 "./include"
                                 REQUIRES DHT22 spi_flash driver esp_adc_cal)
