/*****< scancodes.h >************************************************************/
/*                                                                            */
/*  Scan Codes - Scan code look up table.                                     */
/*                                                                            */
/*  Author:                                                                   */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   11/25/15                 Initial creation.                               */
/******************************************************************************/
#ifndef __SCANCODES_H__
#define __SCANCODES_H__

#include "cmsis_os.h"

uint8_t scan_code(uint8_t ascii);
uint8_t scan_code_modifier(uint8_t ascii);

#endif // __SCANCODES_H__