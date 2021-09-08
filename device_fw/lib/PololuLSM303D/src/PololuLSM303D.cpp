
#include "PololuLSM303D.h"

#include <vector>

namespace lsm303d {

enum class Reg {
  WHO_AM_I = 0x0F,
  CTRL = 0x1F,
  TEMP_OUT = 0x05,
  MAG_OUT = 0x08,
  ACC_OUT = 0x28,
};

namespace counts {
uint8_t TEMP_OUT_READS = 2;
uint8_t MAG_OUT_READS = 6;
uint8_t ACC_OUT_READS = 6;
}  // namespace counts

namespace values {
uint8_t WHO_AM_I_RESULT = 0x49;
std::vector<uint8_t> CTRL_SET = {
    0x00,  // [CTRL_0] Default
    0x37,  // [CTRL_1] ACC: 12.5 Hz Refresh + Enable XYZ
    0x00,  // [CTRL_2] Default
    0x00,  // [CTRL_3] Default
    0x00,  // [CTRL_4] Default
    0xE8,  // [CTRL_5] Mag: 12.5 Hz Refresh + High Resolution
    0x20,  // [CTRL_6] Mag full-scale +/-4 gauss
    0x00,  // [CTRL_7] Cont-conv mode
};
}  // namespace values

Driver::Driver(uint8_t i2c_address, uint32_t i2c_speed)
    : i2c_address(i2c_address), i2c_speed(i2c_speed) {}

void Driver::io_start(void) {
  Wire.beginTransmission(this->i2c_address);
  Wire.setSpeed(this->i2c_speed);
}

lsm303d::Status Driver::io_end(void) {
  uint8_t resp = Wire.endTransmission();
  return (resp == 0) ? Status::STATUS_OK : Status::STATUS_IO_CTRL;
}

lsm303d::Status Driver::io_read(uint8_t reg, size_t count, uint8_t *buffer) {
  Status status = Status::STATUS_OK;

  if (buffer == nullptr) {
    status = Status::STATUS_INVALID_ARG;
  } else {
    this->io_start();
    Wire.write((uint8_t)reg | 0x80);  // add address auto-increment
    status = this->io_end();
  }

  if (status == Status::STATUS_OK) {
    Wire.requestFrom(this->i2c_address, count);

    while (Wire.available() < count)
      ;

    for (size_t i = 0; i < count; i++) {
      buffer[i] = Wire.read();
    }
  }

  return (status == Status::STATUS_OK) ? Status::STATUS_OK
                                       : Status::STATUS_IO_READ;
}

lsm303d::Status Driver::io_write(uint8_t reg, uint8_t data) {
  this->io_start();

  Wire.write((uint8_t)reg);
  Wire.write(data);

  return (this->io_end() == Status::STATUS_OK) ? Status::STATUS_OK
                                               : Status::STATUS_IO_WRITE;
}

lsm303d::Status Driver::init(void) {
  lsm303d::Status status = Status::STATUS_OK;
  if (!Wire.isEnabled()) {
    Wire.begin();
  }
  status = this->sanity_check();

  if (status == Status::STATUS_OK) {
    status = this->set_config();
  }

  return status;
}

lsm303d::Status Driver::sanity_check(void) {
  uint8_t whoami = 0;
  Status status = this->io_read((uint8_t)Reg::WHO_AM_I, 1, &whoami);

  if (status == Status::STATUS_OK) {
    if (whoami != values::WHO_AM_I_RESULT) {
      status = Status::STATUS_INVALID_SIGNATURE;
    }
  }

  return status;
}

Status Driver::set_config(void) {
  Status status = Status::STATUS_OK;

  for (size_t i = 0;
       (status == Status::STATUS_OK) && (i < values::CTRL_SET.size()); i++) {
    status = this->io_write((uint8_t)Reg::CTRL + i, values::CTRL_SET[i]);
  }

  return status;
}

lsm303d::Status Driver::read(lsm303d::data_t *data) {
  Status status = Status::STATUS_OK;
  uint8_t read_buffer[6];

  if (data == nullptr) {
    status = Status::STATUS_INVALID_ARG;
  } else {
    status = this->io_read((uint8_t)Reg::TEMP_OUT, 2, read_buffer);
  }

  if (status == Status::STATUS_OK) {
    data->temp = ((read_buffer[1] & 0x0F) << 8) | read_buffer[0];

    status = this->io_read((uint8_t)Reg::MAG_OUT, counts::MAG_OUT_READS,
                           read_buffer);
  }

  if (status == Status::STATUS_OK) {
    data->mag.x = (read_buffer[1] << 8) | read_buffer[0];
    data->mag.y = (read_buffer[3] << 8) | read_buffer[2];
    data->mag.z = (read_buffer[5] << 8) | read_buffer[4];

    status = this->io_read((uint8_t)Reg::ACC_OUT, counts::ACC_OUT_READS,
                           read_buffer);
  }

  if (status == Status::STATUS_OK) {
    data->acc.x = (read_buffer[1] << 8) | read_buffer[0];
    data->acc.y = (read_buffer[3] << 8) | read_buffer[2];
    data->acc.z = (read_buffer[5] << 8) | read_buffer[4];
  }

  return status;
}

}  // namespace lsm303d
