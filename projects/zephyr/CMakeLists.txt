cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(ccid-serial_project)

target_include_directories(app PRIVATE .)
target_sources(
  app
  PRIVATE ../../src/sample/zephyr/pcsc-serial-sample-main-zephyr.c
          ../../src/sample/pcsc-serial-sample.c
          ../../src/hal/zephyr/zephyr_hal.c
          ../../src/ccid/ccid_convert.c
          ../../src/ccid/ccid_exchange.c
          ../../src/ccid/ccid_helpers.c
          ../../src/ccid/ccid_serial_receiver.c
          ../../src/ccid/ccid_serial_sender.c
          ../../src/scard/scard_core.c
          ../../src/scard/scard_helpers.c)
