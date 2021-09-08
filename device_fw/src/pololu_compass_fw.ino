/*
 * Project pololu_compass_fw
 * Description:
 * Author: Irusha Vidanamadura <irusha@vidanamdaura.net>
 * Date: 2021
 */
#include <Particle.h>
#include <ParticleWebLog.h>
#include <PololuLSM303D.h>

#include <string>

ParticleWebLog particleWebLog;

lsm303d::Driver driver(0x1D, CLOCK_SPEED_100KHZ);

String build_dim_data(lsm303d::dimension_t *dim) {
  int val;
  char buf[64];

  memset(buf, 0, sizeof(buf));
  JSONBufferWriter writer(buf, sizeof(buf) - 1);

  writer.beginObject();
  val = dim->x;
  writer.name("x").value(val);
  val = dim->y;
  writer.name("y").value(val);
  val = dim->z;
  writer.name("z").value(val);
  writer.endObject();

  return String(buf);
}

String publish_compass_data(void) {
  lsm303d::data_t data;

  lsm303d::Status status = driver.read(&data);
  if (status == lsm303d::Status::STATUS_OK) {
    char buf[256];
    memset(buf, 0, sizeof(buf));
    JSONBufferWriter writer(buf, sizeof(buf) - 1);

    writer.beginObject();
    writer.name("success").value(true);
    writer.name("acc").value(build_dim_data(&data.acc));
    writer.name("mag").value(build_dim_data(&data.mag));
    int temp = data.temp;
    writer.name("temp").value(temp);
    writer.endObject();

    return String(buf);
  } else {
    Log.warn("Read data failed: 0x%X", static_cast<unsigned int>(status));

    return "{\"success\": false}";
  }
}

// setup() runs once, when the device is first turned on.
void setup() {
  // Put initialization like pinMode and begin functions here.

  lsm303d::Status status = driver.init();
  if (status == lsm303d::Status::STATUS_OK) {
    Log.info("Driver Setup Success");
    Particle.variable("compass_raw", publish_compass_data);
  } else {
    Log.warn("Driver setup failed: 0x%X", static_cast<unsigned int>(status));
  }
}

// loop() runs over and over again, as quickly as it can execute.
void loop() {
  // The core of your code will likely live here.
}