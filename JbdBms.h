/*
 * \brief Header file for JbdBms class
 * \author rahmaevao
 * \version 0.1
 * \date September 2019
 */

#ifndef JBD_BMS_HPP_
#define JBD_BMS_HPP_

#include "Arduino.h"
//#include <SoftwareSerial.h>

#define BMS_LEN_RESPONCE 34
/* \defgroup BMS_STATUS BMS protection status masks
*  @{
*/
#define BMS_STATUS_OK				0
#define BMS_STATUS_CELL_OVP			1
#define BMS_STATUS_CELL_UVP			2		///< Power off
#define BMS_STATUS_PACK_OVP			4
#define BMS_STATUS_PACK_UVP			8		///< Power off
#define BMS_STATUS_CHG_OTP			16
#define BMS_STATUS_CHG_UTP			32
#define BMS_STATUS_DSG_OTP			64		///< Power off
#define BMS_STATUS_DSG_UTP			128		///< Power off
#define BMS_STATUS_CHG_OCP			256
#define BMS_STATUS_DSG_OCP			512		///< Power off
#define BMS_STATUS_SHORT_CIRCOUT	1024	///< Power off
#define BMS_STATUS_AFE_ERROR		2048
#define BMS_STATUS_SOFT_LOCK		4096
#define BMS_STATUS_CHGOVERTIME		8192
#define BMS_STATUS_DSGOVERTIME		16384	///< Power off
/* @} Power off errors 100011011001010 - 0x46CA*/
#define BMS_POWER_OFF_ERRORS		0x46CA

/**
 * \brief Class for working with BMS, which are compatible with the program
 * JBDTOOLS
 *
 * The sequence of work with the class:
 * 1.
 *
 * \ warning Adout JBD: maximum frequency (min time) data: 22.7 Hz(43.87 ms).
 * There are brakes due to the work of the BMS itself (she didn’t put a trip
 * to DMA UART communication.
 * Probability is that the update data in the BMS itself
 * occurs with a frequency (period) of 5 Hz (200 ms).
*/

typedef struct packCellInfoStruct{
  uint8_t NumOfCells;
  uint16_t CellVoltage[15]; // Max 16 Cell BMS
  uint16_t CellLow;
  uint16_t CellHigh;
  uint16_t CellDiff; // difference between highest and lowest
  uint16_t CellAvg;
} packCellInfoStruct;

class JbdBms {
public:
	void begin(int spd, int rx,int tx);

  //Basin Info
  bool readBmsData();
  float getVoltage();
  float getCurrent();
	float getChargePercentage();
	uint16_t getProtectionState();
  uint16_t getCycle();
  float getTemp1();
  float getTemp2();

  //Pack Info
  packCellInfoStruct getPackCellInfo();
  bool readPackData();

private:
  //SoftwareSerial m_serial;
  float m_voltage = 0;
  float m_current = 0;
  float m_chargePercentage = 0;
  uint16_t m_protectionState = 0;
  uint16_t m_cycle = 0;
  float m_Temp1 = 0;
  float m_Temp2 = 0;


  packCellInfoStruct m_packCellInfo;

  void sendReqBasicMessage();
  void parseReqBasicMessage(uint8_t * t_message);

  void sendCellMessage();


  void parseReqPackMessage(uint8_t * t_message);

  bool readResponce(uint8_t * t_outMessage);

  bool checkCheckSumRecieve(uint8_t * t_message);
  uint16_t computeCrc16JbdChina(uint8_t * puchMsg, uint8_t usDataLen);
  float converUint32ToFloat(uint32_t number);
  //  uint16_t PackCurrent = two_ints_into16(highbyte, lowbyte);
  uint16_t two_ints_into16(int highbyte, int lowbyte);

  uint32_t getMaxTimeout();
};

#endif /* JBD_BMS_HPP_ */
