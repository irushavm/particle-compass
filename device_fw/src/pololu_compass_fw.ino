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

void compass_poll_handler(void);
lsm303d::Driver compass(0x1D, CLOCK_SPEED_100KHZ);
lsm303d::Status compass_data_status = lsm303d::Status::STATUS_FAIL;
lsm303d::data_t compass_data;
Timer compass_poll_timer(500, compass_poll_handler);

void compass_poll_handler(void) {
  SINGLE_THREADED_BLOCK() { compass_data_status = compass.read(&compass_data); }
}

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
  String ret_str;
  lsm303d::data_t data_cp;
  lsm303d::Status data_status;

  SINGLE_THREADED_BLOCK() {
    data_status = compass_data_status;
    data_cp = compass_data;
  }

  if (data_status != lsm303d::Status::STATUS_OK) {
    Log.warn("Read data failed: 0x%X", static_cast<unsigned int>(data_status));
    ret_str = "{\"success\": false}";
  } else {
    char buf[256];
    memset(buf, 0, sizeof(buf));
    JSONBufferWriter writer(buf, sizeof(buf) - 1);

    writer.beginObject();
    writer.name("success").value(true);
    writer.name("acc").value(build_dim_data(&data_cp.acc));
    writer.name("mag").value(build_dim_data(&data_cp.mag));
    int temp = data_cp.temp;
    writer.name("temp").value(temp);
    writer.endObject();

    ret_str = String(buf);
  }

  return ret_str;
}

// setup() runs once, when the device is first turned on.
void setup() {
  // Put initialization like pinMode and begin functions here.

  lsm303d::Status status = compass.init();
  if (status == lsm303d::Status::STATUS_OK) {
    Log.info("Driver Setup Success");
    Particle.variable("compass_raw", publish_compass_data);
    compass_poll_timer.start();
  } else {
    Log.warn("Driver setup failed: 0x%X", static_cast<unsigned int>(status));
  }
}

// loop() runs over and over again, as quickly as it can execute.
void loop() {
  // The core of your code will likely live here.
}