// ************************************************
// Log to SD card
// ************************************************

void LogToSD() {
  unsigned long SD_currentMillis = millis();
  DateTime now = rtc.now();
  pinMode(10, OUTPUT);
  digitalWrite(10, HIGH);
  Serial.println("LOGGING TO SD");

  char* timeFormat = "YYYY/MM/DD/hh/YYMMDDhh.txt";
  char* linesFormat = "YYYY/MM/DD/hh/hh.txt";
  char* formattedFileName = now.toString(timeFormat);
  char* formattedLineFileName = now.toString(linesFormat);
  char* directoryFormat = "YYYY/MM/DD/hh";
  char* formattedDirectoryName = now.toString(directoryFormat);
  Serial.println(formattedDirectoryName);
  Serial.println(formattedFileName);
  
  if (SD_currentMillis - SD_previousMillis >= SD_interval) {
    SD_previousMillis = SD_currentMillis;
    SD.mkdir(formattedDirectoryName);

    myFile = SD.open(formattedFileName, FILE_WRITE);
    myFile.println("time,temp,temp setpoint,pH,pH setpoint");
    myFile.close();
  }

  pinMode(10, OUTPUT);
  digitalWrite(10, HIGH);
  SD.mkdir(formattedDirectoryName);

  // update accompanying line-count file
  File lineFile = SD.open(formattedLineFileName, FILE_WRITE);
  int lineFileSize = lineFile.size();
  if (!lineFileSize) {
    lineFile.println("1");
    lineFile.close();
  } else {
    char linesBuffer[10];
    lineFile.seek(0);
    byte read;
    for (int i = 0; i < 10 && read != -1; i++) {
      read = lineFile.read();
      linesBuffer[i] = (char)read;
    }
    int lineCount = atoi(linesBuffer);
    lineFile.close();
    SD.remove(formattedLineFileName);
    lineFile = SD.open(formattedLineFileName, FILE_WRITE);
    char lineCountBuffer[10];
    sprintf(lineCountBuffer, "%d", lineCount+1);
    lineFile.println(lineCountBuffer);
    lineFile.close();
  }

  myFile = SD.open(formattedFileName, FILE_WRITE);
  // create information string with timestamp
  char formattedSDString[50];
  memset(formattedSDString, 0, 50);
  char timeBuffer[100];
  memset(timeBuffer, 0, 100);
  char varBuffer[100];
  memset(varBuffer, 0, 100);
  DateTime tempNow = rtc.now();
  fmtDouble(tempNow.year(), 0, timeBuffer, 0xffff);
  strcat(formattedSDString, timeBuffer);
  strcat(formattedSDString, "/");
  fmtDouble(tempNow.month(), 0, timeBuffer, 0xffff);
  strcat(formattedSDString, timeBuffer);
  strcat(formattedSDString, "/");
  fmtDouble(tempNow.day(), 0, timeBuffer, 0xffff);
  strcat(formattedSDString, timeBuffer);
  strcat(formattedSDString, " ");
  fmtDouble(tempNow.hour(), 0, timeBuffer, 0xffff);
  strcat(formattedSDString, timeBuffer);
  strcat(formattedSDString, ":");
  if (tempNow.minute() < 10) {
    strcat(formattedSDString, "0");
  }
  fmtDouble(tempNow.minute(), 0, timeBuffer, 0xffff);
  strcat(formattedSDString, timeBuffer);
  strcat(formattedSDString, ":");
  if (tempNow.second() < 10) {
    strcat(formattedSDString, "0");
  }
  fmtDouble(tempNow.second(), 0, timeBuffer, 0xffff);
  strcat(formattedSDString, timeBuffer);
  strcat(formattedSDString, ":");
  fmtDouble(millis(), 0, timeBuffer, 0xffff);
  strcat(formattedSDString, timeBuffer);

  snprintf(varBuffer, 100, "%d", tankid);
  strcat(formattedSDString, ",");
  strcat(formattedSDString, varBuffer);
  fmtDouble(temp, 3, varBuffer, 0xffff);
  strcat(formattedSDString, ",");
  strcat(formattedSDString, varBuffer);
  fmtDouble(tempset, 3, varBuffer, 0xffff);
  strcat(formattedSDString, ",");
  strcat(formattedSDString, varBuffer);
  fmtDouble(pH, 3, varBuffer, 0xffff);
  strcat(formattedSDString, ",");
  strcat(formattedSDString, varBuffer);
  fmtDouble(phset, 3, varBuffer, 0xffff);
  strcat(formattedSDString, ",");
  strcat(formattedSDString, varBuffer);
  fmtDouble(onTime, 3, varBuffer, 0xffff);
  strcat(formattedSDString, ",");
  strcat(formattedSDString, varBuffer);
  fmtDouble(Kp, 3, varBuffer, 0xffff);
  strcat(formattedSDString, ",");
  strcat(formattedSDString, varBuffer);
  fmtDouble(Ki, 3, varBuffer, 0xffff);
  strcat(formattedSDString, ",");
  strcat(formattedSDString, varBuffer);
  fmtDouble(Kd, 3, varBuffer, 0xffff);
  strcat(formattedSDString, ",");
  strcat(formattedSDString, varBuffer);

  myFile.println(formattedSDString);
  myFile.close();
  Serial.println(formattedSDString);
}