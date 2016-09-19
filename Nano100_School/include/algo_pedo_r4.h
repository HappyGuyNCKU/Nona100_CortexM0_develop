#ifndef ALGO_PEDO_R4_H
#define ALGO_PEDO_R4_H

int pedoDect_R4_open();
int pedoDect_R4_process(int raw_valueX, int raw_valueY, int raw_valueZ);
int pedoDect_R4_detection(int filter_value);
int pedoDect_R4_close();

#endif
