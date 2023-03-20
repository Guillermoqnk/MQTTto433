#include <time.h>    
#include <ESP8266WiFi.h>
#ifndef COOGLEIOT_TIMEZONE_OFFSET
#define COOGLEIOT_TIMEZONE_OFFSET ((3600 * 1) ) // Default Timezone is -5 UTC (America/New York)
#endif

#ifndef COOGLEIOT_DAYLIGHT_OFFSET
#define COOGLEIOT_DAYLIGHT_OFFSET 0 // seconds
#endif

#ifndef COOGLEIOT_NTP_SERVER_1
#define COOGLEIOT_NTP_SERVER_1 "pool.ntp.org"
#endif

#ifndef COOGLEIOT_NTP_SERVER_2
#define COOGLEIOT_NTP_SERVER_2 "time.nist.gov"
#endif

#ifndef COOGLEIOT_NTP_SERVER_3
#define COOGLEIOT_NTP_SERVER_3 "time.google.com"
#endif
time_t now;      

void syncNTPTime(void)
{
 if(!WiFi.status() == WL_CONNECTED) {
    Serial.println("Cannot synchronize time with NTP Servers - No WiFi Connection");
  }



  configTime(COOGLEIOT_TIMEZONE_OFFSET, COOGLEIOT_DAYLIGHT_OFFSET, COOGLEIOT_NTP_SERVER_1, COOGLEIOT_NTP_SERVER_2, COOGLEIOT_NTP_SERVER_3);
 
  for(int i = 0; (i < 100) && (50000 >time(nullptr)); i++) {
    delay(100);
  }

  if(50000 >(now = time(nullptr))) {
    Serial.println("Failed to synchronize with time server!");
  } else {
    Serial.println("Time successfully synchronized with NTP server");

  }
}



String getTimestampAsString()
{
  String timestamp;
  struct tm* p_tm;
  now = time(nullptr);
   Serial.print("ctime: ");
  Serial.println(ctime(&now));
  
  if(now) {
    p_tm = localtime(&now); 
    //Serial.print("ctime: ");
    //Serial.println(asctime(p_tm));
    //char cadena[80];
    //strftime (cadena,80,"Now it's %I:%M%p.",p_tm);
    //Serial.println(cadena);
    timestamp = timestamp +
            (p_tm->tm_year + 1900) + "-" +
            (p_tm->tm_mon < 10 ? "0" : "") + p_tm->tm_mon + "-" +
          (p_tm->tm_mday < 10 ? "0" : "") + p_tm->tm_mday + " " +
          (p_tm->tm_hour < 10 ? "0" : "") + p_tm->tm_hour + ":" +
          (p_tm->tm_min < 10 ? "0" : "") + p_tm->tm_min + ":" +
          (p_tm->tm_sec < 10 ? "0" : "") + p_tm->tm_sec;
  } else {
    timestamp = F("UKWN");
  }

  return timestamp;
}
