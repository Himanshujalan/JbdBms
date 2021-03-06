/*
 * \brief Source file for JbdBms class
 * \author rahmaevao
 * \version 0.1
 * \date September 2019
 */

#include "jbdBms.h"

void JbdBms::begin(int spd, int rx,int tx) {
  //m_serial.begin(9600);
  Serial2.begin(spd, SERIAL_8N1, rx, tx);
}

bool JbdBms::readBmsData(){
  uint8_t responce[BMS_LEN_RESPONCE];

  sendReqBasicMessage();
  readResponce(responce);

  if (checkCheckSumRecieve(responce) == true) {
    parseReqBasicMessage(responce);
  } else {
    return false;
  }
  return true;
}

float JbdBms::getVoltage() {
  return m_voltage;
}

float JbdBms::getCurrent() {
  return m_current;
}

float JbdBms::getChargePercentage() {
  return m_chargePercentage;
}
uint16_t JbdBms::getProtectionState() {
  return m_protectionState;
}

uint16_t JbdBms::getCycle(){
  return m_cycle;
}


/**
 * \External Temp Probe
 */
float JbdBms::getTemp1() {
  return m_Temp1;
}

/**
 * \Onboard Temp
 */
float JbdBms::getTemp2() {
  return m_Temp2;
}

bool JbdBms::readPackData(){
  uint8_t responce[BMS_LEN_RESPONCE];

  sendCellMessage();
  readResponce(responce);

  if (checkCheckSumRecieve(responce) == true) {
    parseReqPackMessage(responce);
  } else {
    return false;
  }
  return true;
}

/**
 * \Return  packCellInfoStruct
 * \NumOfCells;
 * \CellVoltage[15]; // Max 16 Cell BMS
 * \CellHigh;
 * \CellLow;
 * \CellDiff; // difference between highest and lowest
 * \CellAvg;
 */
packCellInfoStruct JbdBms::getPackCellInfo() {
  return m_packCellInfo;
}

/**
 * \Basic Capacity, current, protection state
 */
void JbdBms::sendReqBasicMessage() {
  uint8_t reqMessage[] = { 0xDD, 0xA5, 0x03, 0x00, 0xFF, 0xFD, 0x77 };
  //m_serial.write(reqMessage, 7);
  Serial2.write(reqMessage, 7);
}

/**
 * \Individual cell info
 */
void JbdBms::sendCellMessage() {
  uint8_t reqMessage[] = { 0xDD, 0xA5, 0x04, 0x00, 0xFF, 0xFC, 0x77 };
  //m_serial.write(reqMessage, 7);
  Serial2.write(reqMessage, 7);
}

void JbdBms::parseReqBasicMessage(uint8_t * t_message) {
  //m_current = ((t_message[6] << 8) | t_message[7]) * 10;
  m_voltage = (float)two_ints_into16(t_message[4], t_message[5])/100;
  m_current = ((float)two_ints_into16(t_message[6], t_message[7])) * 10;

  m_chargePercentage = t_message[23];

  //m_protectionState = (t_message[20] << 8) | t_message[21];
  m_protectionState = ((float)two_ints_into16(t_message[20], t_message[21]));

  m_cycle = ((float)two_ints_into16(t_message[12], t_message[13]));

  m_Temp1 = (((float)two_ints_into16(t_message[27], t_message[28])) - 2731) / 10.00f;
  m_Temp2 = (((float)two_ints_into16(t_message[29], t_message[30])) - 2731) / 10.00f;
  
}

void JbdBms::parseReqPackMessage(uint8_t * t_message){ //packCellInfoStruct * t_packCellInfo) {
  uint16_t _cellSum = 0;
  uint16_t _CellLow = 5000; //5v
  uint16_t _CellHigh = 0;


  //uint8_t reqMessage[] = { 0xDD, 0xA5, 0x00, 0x1E, 0x0F, 0x66, 0x0F, 0x63, 0x0F, 0x63, 0x0F, 0x64, 0x0F, 0x3E, 0x0F, 0x63, 0x0F, 0x37, 0x0F, 0x5B, 0x0F, 0x65, 0x0F, 0x3B, 0x0F, 0x63, 0x0F, 0x63, 0x0F, 0x3C, 0x0F, 0x66, 0x0F, 0x3D, 0xF9, 0xF9, 0x77};
  m_packCellInfo.NumOfCells = t_message[3] / 2;  //Data length * 2 is number of cells !!!!!!

  //go trough individual cells

  byte offset = 4;
  //go trough individual cells
  for (byte i = 0; i < m_packCellInfo.NumOfCells; i++)
  {
    m_packCellInfo.CellVoltage[i] = ((uint16_t)two_ints_into16(t_message[i * 2 + offset], t_message[i * 2 + 1 + offset])); //Data length * 2 is number of cells !!!!!!

    _cellSum += m_packCellInfo.CellVoltage[i];

    if (m_packCellInfo.CellVoltage[i] > _CellHigh)
    {
        _CellHigh = m_packCellInfo.CellVoltage[i];
    }
    if (m_packCellInfo.CellVoltage[i] < _CellLow)
    {
        _CellLow = m_packCellInfo.CellVoltage[i];
    }

    m_packCellInfo.CellLow = _CellLow;
    m_packCellInfo.CellHigh = _CellHigh;
    m_packCellInfo.CellDiff = _CellHigh - _CellLow; // Resolution 10 mV -> convert to volts
    m_packCellInfo.CellAvg = _cellSum / m_packCellInfo.NumOfCells;

  }
}

bool JbdBms::readResponce(uint8_t * t_outMessage) {
  uint8_t i = 0;
  bool findBeginByte = false;
  uint32_t statrTime = millis();
  while (i <= BMS_LEN_RESPONCE - 1) {
    if (abs((millis()-statrTime) > getMaxTimeout()))
    {
      return false;
    }
    //if (m_serial.available() > 0) {
    if (Serial2.available() > 0) {
      //uint8_t thisByte = m_serial.read();
      uint8_t thisByte = Serial2.read();
      if (thisByte == 0xDD)
      {
        findBeginByte = true;
      }
      if (findBeginByte) {
        t_outMessage[i] = thisByte;
        i++;
      }
    }
  }
  return true;
}

/**
 * \Compute and cheking check sum in message
 * \param [in] t_message
 * \return Fact of passing the test:
 *  true - check passed;
 *  false - check not passed
 */
bool JbdBms::checkCheckSumRecieve(uint8_t * t_message) {

  uint16_t checkSumCompute;
  uint16_t checkSumAccepted;
  uint8_t lengthData;
  uint8_t startIndexCS;

  //uint8_t status = t_message[3];

  if (t_message[2] != 0) // Status OK
    return false;

  lengthData = t_message[3];
  checkSumCompute = computeCrc16JbdChina(t_message, BMS_LEN_RESPONCE);
  startIndexCS = lengthData + 4;

  checkSumAccepted = (t_message[startIndexCS] << 8)
      | t_message[startIndexCS + 1];

  if (checkSumCompute != checkSumAccepted)
    return false;
  return true;
}

/**
 * \Compute check sum in message for JBD protocol semiModbus :)
 * \param[in] puchMsg Message buffer containing binary data to be used for
 *            generating the CRC.
 * \param[in] usDataLen The quantity of bytes in the message buffer.
 * \return The function returns the CRC.
 */
uint16_t JbdBms::computeCrc16JbdChina(uint8_t * puchMsg, uint8_t usDataLen) {
  uint8_t lengthData = puchMsg[3];

  uint16_t summa = 0;

  for (int i = 4; i < lengthData + 4; i++)
    summa = summa + puchMsg[i];

  uint16_t checkSum = (summa + lengthData - 1) ^ 0xFFFF;
  return checkSum;
}

/**
 * \ Build a float from uint32_t
 */
float JbdBms::converUint32ToFloat(uint32_t number) {
  union DataType {
    float f;
    uint32_t uint32t;
  };

  union DataType sample;
  sample.uint32t = number;
  return sample.f;
}

uint32_t JbdBms::getMaxTimeout(){
  return 100;
}


//------------------------------------------------------------------------------------------
//  uint16_t PackCurrent = two_ints_into16(highbyte, lowbyte);
uint16_t JbdBms::two_ints_into16(int highbyte, int lowbyte) // turns two bytes into a single long integer
{
  uint16_t a16bitvar = (highbyte);
  a16bitvar <<= 8; //Left shift 8 bits,
  a16bitvar = (a16bitvar | lowbyte); //OR operation, merge the two
  return a16bitvar;
}
