/*
  Projet ESP8266 Enviro Monitor Station
  Copyright (C) 2017 by Leon

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

//#include <SoftwareSerial.h>  //软件模拟第二个串口

/************* ch2o *******************************/
float ch2o_value;
//const byte rxPinCh2o = D7;
//const byte txPinCh2o = D8;
//SoftwareSerial mySerialCh2o (rxPinCh2o, txPinCh2o);

/*void setup()
{
  Serial.begin(9600);
  mySerialCh2o.begin(9600);
  while (!mySerialCh2o)  {  }// wait for serial line to be ready
}
*/
void check_ch2o()
{
  uint8_t mySerialBuffer[9] = {0};
  long ch2o_value_dot = 1;
  uint8_t ch2o_verify = 0;

  //读取串口缓冲区
  int available_num = Serial.available();

  if (mySerialCh2o.available() > 0)
  {
    available_num = mySerialCh2o.available();
    if (available_num = 9)
    {
      for (int a = 0; a < 9; a++)
      {
        mySerialBuffer[a] = mySerialCh2o.read();
      }
      ch2o_verify = (uint8_t)(mySerialBuffer[2] + mySerialBuffer[3] + mySerialBuffer[4] + mySerialBuffer[5] + mySerialBuffer[6] + mySerialBuffer[7]);


      if (ch2o_verify == mySerialBuffer[8])
      {
        while (mySerialBuffer[6]--)ch2o_value_dot = ch2o_value_dot * 10;
        if ((mySerialBuffer[4] * 256 + mySerialBuffer[5]) != 0)
          ch2o_value = (float)(mySerialBuffer[4] * 256 + mySerialBuffer[5]) / (float)ch2o_value_dot;
        Serial.print("Ch2o Value Is: ");
        Serial.print(ch2o_value);
        Serial.println(" mg/m3");
        client.publish(ch2o_topic, String(ch2o_value).c_str(), true);
      }
      else
      {
        available_num = mySerialCh2o.available();
        for (int a = 0; a < available_num; a++)
        {
          mySerialCh2o.read();
        }
      }

    }
    else if (available_num > 9)
    {
      available_num = mySerialCh2o.available();
      for (int a = 0; a < available_num; a++)
      {
        mySerialCh2o.read();
      }
    }
  }
}
