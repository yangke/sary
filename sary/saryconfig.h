#ifndef __SARY_CONFIG_H__
#define __SARY_CONFIG_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*
 * Use SaryInt instead of gint for ease to support 64 bit
 * array in future. Note: GINT_FROM_BE, GINT_TO_BE must also
 * be replaced for 64 bit support.
 */
typedef int SaryInt;

extern const int sary_major_version;
extern const int sary_minor_version;
extern const int sary_micro_version;

#define SARY_MAJOR_VERSION 1
#define SARY_MINOR_VERSION 2
#define SARY_MICRO_VERSION 0

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __SARY_CONFIG_H__ */
