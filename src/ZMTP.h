#ifndef __ZMTP_H_INCLUDED__
#define __ZMTP_H_INCLUDED__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _zmtp_frame_t zmtp_frame_t;
typedef struct _zmtp_msg_t zmtp_msg_t;

zmtp_frame_t * zmtp_frame_new();
void zmtp_frame_destroy(zmtp_frame_t **self_p);

zmtp_msg_t * zmtp_msg_new();
void zmtp_msg_destroy(zmtp_msg_t **self_p);

#ifdef __cplusplus
}
#endif

#endif
