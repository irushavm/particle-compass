
#ifndef POLOLU_LSM303D_H
#define POLOLU_LSM303D_H

#include <cstdint>

// Particle includes
#include <Particle.h>

namespace lsm303d {

enum class Status {
  STATUS_OK = 0,
  STATUS_FAIL,
  STATUS_INVALID_ARG,
  STATUS_IO_CTRL,
  STATUS_IO_READ,
  STATUS_IO_WRITE,
  STATUS_INVALID_SIGNATURE,

};

typedef struct {
  int16_t x;
  int16_t y;
  int16_t z;
} dimension_t;

/**
 * @brief Information to be received from the device read
 *
 */
typedef struct {
  int16_t temp;
  dimension_t mag;  // magnetometer
  dimension_t acc;
} data_t;

class Driver {
 public:
  Driver(uint8_t i2c_address, uint32_t i2c_speed);

  Status init();
  Status read(lsm303d::data_t *data);

 private:
  uint8_t i2c_address;
  uint32_t i2c_speed;

  void io_start(void);
  lsm303d::Status io_end(void);
  lsm303d::Status io_read(uint8_t reg, size_t count, uint8_t *buffer);
  lsm303d::Status io_write(uint8_t reg, uint8_t data);

  Status sanity_check(void);
  Status set_config(void);
};

}  // namespace lsm303d

#endif  // POLOLU_LSM303D_H