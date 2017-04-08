#ifndef FACE_MATCHING_H_INCLUDED
#define FACE_MATCHING_H_INCLUDED

#include <stdint.h>

enum NStatusCodes
{
	NST_ERROR = -1,
	NST_MISMATCH = 0,
	NST_MATCH = 1
};

/*
 * Matching of two templates
 *
 * Arguments:
 *		probeBuff      - Pointer to memory area where probe NLRecord is saved
 *		probeLength    - Length of probe memory area in bytes
 *		templateBuff   - Pointer to memory area where enrolled NLRecord is saved
 *		templateLength - Length of template memory area in bytes
 *		matchThreshold - Matching threshold
 *
 * Return:
 *		One value of NStatusCodes
 */
enum NStatusCodes NFaceMatch(const void * probeBuff, int32_t probeLength, const void * templateBuff, int32_t templateLength, int32_t matchThreshold);

#endif
