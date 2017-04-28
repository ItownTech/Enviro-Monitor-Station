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

#define LENG 32
char buf[LENG];
int PM01Value = 0;        //define PM1.0 value of the air detector module
int PM2_5Value = 0;       //define PM2.5 value of the air detector module
int PM10Value = 0;        //define PM10 value of the air detector module

/*
  void check_pm()
  {
  int i = 0;
  check1_pm();
  for (i = 0 ; i < 61; i++) {
    if (i != 60)
    {

      digitalWrite(RELAY_PIN, HIGH);
      check1_pm();
      //     Serial.print(i);
      //     Serial.println(" | led on");

    }
    else
    {
      digitalWrite(RELAY_PIN, LOW);
      //     Serial.print(i);
      //     Serial.println(" | led off");
      //     delay(1000 * 10);
      delay(1000 * 60 * 10);
    }
  }
  }

*/
void check_pm()
{
  //  relay();
  if (Serial.available())
  {
    Serial.readBytes(buf, LENG);
    if (buf[0] == 0x42 && buf[1] == 0x4d) {
      if (checkValue(buf, LENG)) {
        PM01Value = transmitPM01(buf); //count PM1.0 value of the air detector module
        PM2_5Value = transmitPM2_5(buf); //count PM2.5 value of the air detector module
        PM10Value = transmitPM10(buf); //count PM10 value of the air detector module
      }
    }
  }
  static unsigned long OledTimer = millis();
  if (millis() - OledTimer >= 1000)
  {
    OledTimer = millis();

    Serial.print("PM 1.0: ");  //send PM1.0 data to bluetooth
    Serial.print(PM01Value);
    Serial.print("  ug/m3");

    Serial.print(" | PM 2.5: ");  //send PM1.0 data to bluetooth
    Serial.print(PM2_5Value);
    Serial.print("  ug/m3");

    Serial.print(" | PM 10:  ");  //send PM1.0 data to bluetooth
    Serial.print(PM10Value);
    Serial.println("  ug/m3");
    client.publish(pm1_topic, String(PM01Value).c_str(), true);                       //sent message to MQTT
    client.publish(pm2_5_topic, String(PM2_5Value).c_str(), true);                     //sent message to MQTT
    client.publish(pm10_topic, String(PM10Value).c_str(), true);                      //sent message to MQTT
  }

}
char checkValue(char *thebuf, char leng)
{
  char receiveflag = 0;
  int receiveSum = 0;
  char i = 0;

  for (i = 0; i < leng; i++)
  {
    receiveSum = receiveSum + thebuf[i];
  }

  if (receiveSum == ((thebuf[leng - 2] << 8) + thebuf[leng - 1] + thebuf[leng - 2] + thebuf[leng - 1])) //check the serial data
  {
    receiveSum = 0;
    receiveflag = 1;
  }
  return receiveflag;
}

int transmitPM01(char *thebuf)
{
  int PM01Val;
  PM01Val = ((thebuf[10] << 8) + thebuf[11]); //count PM1.0 value of the air detector module
  return PM01Val;
}

//transmit PM Value to PC
int transmitPM2_5(char *thebuf)
{
  int PM2_5Val;
  PM2_5Val = ((thebuf[12] << 8) + thebuf[13]); //count PM2.5 value of the air detector module
  return PM2_5Val;
}

//transmit PM Value to PC
int transmitPM10(char *thebuf)
{
  int PM10Val;
  PM10Val = ((thebuf[14] << 8) + thebuf[15]); //count PM10 value of the air detector module
  return PM10Val;
}
