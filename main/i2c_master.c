#include "i2c_master.h"

#include "epd_board.h"

#include "driver/i2c.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define ACK_ENABLE false

esp_err_t i2c_master_init(i2c_port_t i2c_num, int sda_num, int scl_num) {
    i2c_config_t i2c_conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = sda_num,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = scl_num,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };
    i2c_param_config(i2c_num, &i2c_conf);
    return i2c_driver_install(i2c_num, i2c_conf.mode, 0, 0, 0);
}

esp_err_t i2c_master_read_slave(i2c_port_t i2c_num,
                                uint8_t i2c_slave_addr,
                                uint8_t *data_rd,
                                size_t size,
                                int reg) {
    if (size == 0) {
        return ESP_OK;
    }
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (i2c_slave_addr << 1) | I2C_MASTER_WRITE, ACK_ENABLE);
    i2c_master_write_byte(cmd, reg, ACK_ENABLE);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(i2c_num, cmd, 1000 / portTICK_PERIOD_MS);
    if (ret != ESP_OK) {
        return ret;
    }
    i2c_cmd_link_delete(cmd);
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (i2c_slave_addr << 1) | I2C_MASTER_READ, ACK_ENABLE);
    if (size > 1) {
        i2c_master_read(cmd, data_rd, size - 1, I2C_MASTER_ACK);
    }
    i2c_master_read_byte(cmd, data_rd + size - 1, I2C_MASTER_NACK);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(i2c_num, cmd, 1000 / portTICK_PERIOD_MS);
    if (ret != ESP_OK) {
        return ret;
    }
    i2c_cmd_link_delete(cmd);
    return ESP_OK;
}

esp_err_t i2c_master_write_slave(i2c_port_t i2c_num,
                                 uint8_t i2c_slave_addr,
                                 uint8_t ctrl,
                                 uint8_t *data_wr,
                                 size_t size) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (i2c_slave_addr << 1) | I2C_MASTER_WRITE, ACK_ENABLE);
    i2c_master_write_byte(cmd, ctrl, ACK_ENABLE);

    i2c_master_write(cmd, data_wr, size, ACK_ENABLE);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(i2c_num, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
}

esp_err_t i2c_master_del(i2c_port_t i2c_num) {
    return i2c_driver_delete(i2c_num);
}