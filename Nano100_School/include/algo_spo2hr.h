#ifndef ALGO_SPO2HR_H
#define ALGO_SPO2HR_H

int spo2HRDect_open();
int spo2HRDect_process(int raw_value);
int spo2HRDect_detection(int filter_value);
int spo2HRDect_close();

#endif
