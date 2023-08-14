#ifndef _EI_H_
#define _EI_H_

#define EI_STACK_SIZE 8192
#define EI_THREAD_PRIORITY 4

int k_ei(void);
bool ei_fill_feature_buffer_cb(float*sample);

#endif