#ifndef NBFC_CONFIG_RATING_RULES_DEFAULT_H_
#define NBFC_CONFIG_RATING_RULES_DEFAULT_H_

/* Auto generated code ['./tools/config_rating_rules.py', 'c_code'] */

#define CONFIG_RATING_DEFAULT_RULES \
"{\"FanRegisterFullMatch\":[{\"Name\":\"PFAN\",\"Mode\":\"rw\",\"WritePriority\":100},{\"Name\":\"SFAN\",\"Mode\":\"rw\"},{\"Name\":\"CFAN\",\"Mode\":\"rw\",\"ReadPriority\":100},{\"Name\":\"XFAN\",\"Mode\":\"rw\"},{\"Name\":\"FAN1\",\"Mode\":\"rw\"},{\"Name\":\"FSW1\",\"Mode\":\"rw\",\"WritePriority\":100},{\"Name\":\"RTMP\",\"Mode\":\"rw\",\"Notice\":\"Temperature register; may cause CPU throttling or emergency shutdown\"},{\"Name\":\"GPUF\",\"Mode\":\"rw\"},{\"Name\":\"CPUF\",\"Mode\":\"rw\"},{\"Name\":\"C1FR\",\"Mode\":\"r\"},{\"Name\":\"G1FR\",\"Mode\":\"r\"},{\"Name\":\"FRDC\",\"Mode\":\"r\",\"ReadPriority\":100},{\"Name\":\"FTGC\",\"Mode\":\"w\",\"WritePriority\":100},{\"Name\":\"FR2C\",\"Mode\":\"r\",\"ReadPriority\":100},{\"Name\":\"FT2C\",\"Mode\":\"w\",\"WritePriority\":100}],\"FanRegisterPartialMatch\":[\"FAN\",\"RPM\",\"PWM\"],\"RegisterWriteFullMatch\":[\"TEMP\",\"CRZN\",\"FSH1\",\"FASO\"],\"RegisterWritePartialMatch\":[],\"BadRegisterFullMatch\":[\"FBCM\",\"FBGI\",\"FBAE\",\"FBCB\",\"FBW1\",\"FBW2\",\"FBID\",\"FUAE\",\"FRPS\"],\"BadRegisterPartialMatch\":[\"BAT\"]}"

#endif
