#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <NEO_M6_cmd_parser.h>

/*
#define NEO_BUFF_LEN 10
#define NEO_HEADER_LEN 6
#define NUM_FIELDS 20
#define FIELD_LEN    10
#define HEADER_LEN      6
*/

#define NMEA_ID_POS    7
#define DDC_POS        8
#define SERIAL_1_POS   9
#define SERIAL_2_POS   10
#define USB_POS        11
#define SPI_POS        12

#define BAUD_0   14
#define BAUD_1   15
#define BAUD_2   16
#define BAUD_3   17

#define MEAS_RATE_1   6
#define MEAS_RATE_2   7
#define NAV_RATE_1    8
#define NAV_RATE_2    9
/*
const nmea_conf_package defultConfigPacket = {
	.payload = {
		0xB5, // Header char 1
		0x62, // Header char 2
		0x06, // class
		0x01, // id
		0x08, // length LSB
		0x00, // length MSB
		0xF0, // payload (NMEA sentence ID char 1)
		0x00, // payload (NMEA sentence ID char 2)
		0x00, // payload I/O Target 0 - DDC           - (1 - enable sentence, 0 - disable)
		0x00, // payload I/O Target 1 - Serial Port 1 - (1 - enable sentence, 0 - disable)
		0x00, // payload I/O Target 2 - Serial Port 2 - (1 - enable sentence, 0 - disable)
		0x00, // payload I/O Target 3 - USB           - (1 - enable sentence, 0 - disable)
		0x00, // payload I/O Target 4 - SPI           - (1 - enable sentence, 0 - disable)
		0x00, // payload I/O Target 5 - Reserved      - (1 - enable sentence, 0 - disable)
		0x00, // CK_A
		0x00  // CK_B
	}
};
*/
const char CFG_MSG[NMEA_LEN] = {
	0xB5, // Header char 1
	0x62, // Header char 2
	0x06, // class
	0x01, // id
	0x08, // length LSB
	0x00, // length MSB
	0xF0, // payload (NMEA sentence ID char 1)
	0x00, // payload (NMEA sentence ID char 2)
	0x00, // payload I/O Target 0 - DDC           - (1 - enable sentence, 0 - disable)
	0x00, // payload I/O Target 1 - Serial Port 1 - (1 - enable sentence, 0 - disable)
	0x00, // payload I/O Target 2 - Serial Port 2 - (1 - enable sentence, 0 - disable)
	0x00, // payload I/O Target 3 - USB           - (1 - enable sentence, 0 - disable)
	0x00, // payload I/O Target 4 - SPI           - (1 - enable sentence, 0 - disable)
	0x00, // payload I/O Target 5 - Reserved      - (1 - enable sentence, 0 - disable)
	0x00, // CK_A
	0x00  // CK_B
};


/*
const char CFG_RATE[FREQ_LEN] = {
	0xB5, // sync char 1
	0x62, // sync char 2
	0x06, // class
	0x08, // id
	0x06, // length LSB
	0x00, // length MSB
	0x64, // payload measRate (ms) 1
	0x00, // payload measRate (ms) 2
	0x00, // payload navRate (cycles) 1
	0x00, // payload navRate (cycles) 2
	0x01, // payload timeRef 1
	0x00, // payload timeRef 2
	0x00, // CK_A
	0x00  // CK_B
};

const char CFG_PRT[BAUD_LEN] = {
	0xB5, // sync char 1
	0x62, // sync char 2
	0x06, // class
	0x00, // id
	0x14, // length LSB
	0x00, // length MSB
	0x01, // payload portID
	0x00, // payload reserved0
	0x00, // payload txReady 1
	0x00, // payload txReady 2
	0xD0, // payload mode 1
	0x08, // payload mode 2
	0x00, // payload mode 3
	0x00, // payload mode 4
	0x00, // payload baudRate 0 (LSB)
	0x00, // payload baudRate 1
	0x00, // payload baudRate 2
	0x00, // payload baudRate 3 (MSB)
	0x07, // payload inProtoMask 1
	0x00, // payload inProtoMask 2
	0x03, // payload outProtoMask 1
	0x00, // payload outProtoMask 2
	0x00, // payload reserved4 1
	0x00, // payload reserved4 2
	0x00, // payload reserved5 1
	0x00, // payload reserved5 2
	0x00, // CK_A
	0x00  // CK_B
};
*/
const char GPGGA_header[] = { '$', 'G', 'P', 'G', 'G', 'A' };
const char GPGLL_header[] = { '$', 'G', 'P', 'G', 'L', 'L' };
const char GPGLV_header[] = { '$', 'G', 'P', 'G', 'L', 'V' };
const char GPGSA_header[] = { '$', 'G', 'P', 'G', 'S', 'A' };
const char GPRMC_header[] = { '$', 'G', 'P', 'R', 'M', 'C' };
const char GPVTG_header[] = { '$', 'G', 'P', 'V', 'T', 'G' };

struct {
	int utc_year;
	uint8_t utc_month;
	uint8_t utc_day;
	uint8_t utc_hour;
	uint8_t utc_min;
	float utc_sec;
	float lat;
	float lon;
	float sog;
	float cog_true;
	char navStatus;
	int fs;
	float nosv;
	float hdop;
	float msl;
	char umsl;
	float altref;
	char usep;
	float diffage;
	int diffstation;
} GPS_data;

static uint8_t crc;

static void insertChecksum(char packet[], const uint8_t len) {
	uint8_t ck_a = 0;
	uint8_t ck_b = 0;

	// exclude the first and last two bytes in packet
	for (int i = 2; i < (len - 2); i++)
	{
		ck_a += packet[i];
		ck_b += ck_a;
	}

	packet[len - 2] = ck_a;
	packet[len - 1] = ck_b;
}
/*
void neo6mGPS_changeBaud(uint32_t baud)
{
	char configPacket[BAUD_LEN];
	memcpy(configPacket, CFG_PRT, BAUD_LEN);

	configPacket[BAUD_0] = (char)(baud & 0xFF);
	configPacket[BAUD_1] = (char)((baud >> 8) & 0xFF);
	configPacket[BAUD_2] = (char)((baud >> 16) & 0xFF);
	configPacket[BAUD_3] = (char)((baud >> 24) & 0xFF);

	insertChecksum(configPacket, BAUD_LEN);
	//sendPacket(configPacket, BAUD_LEN);

	//delay(100);

	if (usingUSB)
	{
		usb_port->flush();
		usb_port->begin(baud);
	}
	else
	{
		_port->flush();
		_port->begin(baud);
	}

}



void neo6mGPS_changeFreq(uint16_t hertz)
{
	uint16_t normHerz = hertz / (1000 / ((CFG_RATE[MEAS_RATE_2] << 8) | CFG_RATE[MEAS_RATE_1]));
	char configPacket[FREQ_LEN];
	memcpy(configPacket, CFG_RATE, FREQ_LEN);

	configPacket[NAV_RATE_1] = (char)(normHerz & 0xFF);
	configPacket[NAV_RATE_2] = (char)((normHerz >> 8) & 0xFF);

	insertChecksum(configPacket, FREQ_LEN);
	//sendPacket(configPacket, FREQ_LEN);
}

*/

/*
bool neo6mGPS_available()
{
	char recChar;
	bool endProcessing;

	{
		while (_port->available())
		{
			recChar = _port->read();
			endProcessing = parseData(recChar);

			if (endProcessing)
				return true;
		}
	}

	return false;
}
*/

/*

static bool parseData(char recChar)
{
	if (recChar == '\n')
	{
		startByteFound = false;
		fieldNum = 0;
		fieldIndex = 0;
		updateValues();
			
		return true;
	}
	else if (((recChar == ',') && startByteFound) || ((recChar == '*') && startByteFound))
	{
		fieldNum++;
		fieldIndex = 0;
	}
	else if (!startByteFound)
	{
		if (recChar == '$')
		{
			startByteFound = true;
			data[fieldNum][fieldIndex] = recChar;
			fieldIndex++;
		}
	}
	else
	{
		if ((fieldNum < NUM_FIELDS) && (fieldIndex < FIELD_LEN))
		{
			data[fieldNum][fieldIndex] = recChar;
			fieldIndex++;
		}
	}

	return false;
}

*/

/*
// hhmmss.sss
static void calc_utc_time(char * time)
{
	GPS_data.utc_hour = itoa(time)(int)(time / 10000);
	GPS_data.utc_min = (int)((time - (GPS_data.utc_hour * 10000)) / 100);
	GPS_data.utc_sec = time - (GPS_data.utc_hour * 10000) - (GPS_data.utc_min * 100);
}




// ddmmyy
static void calc_utc_date(int date)
{
	GPS_data.utc_day = date / 10000;
	GPS_data.utc_month = (date - (GPS_data.utc_day * 10000)) / 100;
	GPS_data.utc_year = 2000 + date - (GPS_data.utc_day * 10000) - (GPS_data.utc_month * 100);
}
*/



// (d)ddmm.mmmm, D
static float dm_dd(float loc, char dir)
{
	float result = (int)(loc / 100);

	result += (loc - (result * 100)) / 60.0;

	if (dir == 'S' || dir == 'W')
		result = -result;

	return result;
}

// copy until delimiter
static void getField(char * newMsg, char * buf) {
	static char * msg = NULL;
	
	if(newMsg != NULL) {
		msg = newMsg;
		crc = 0;
	}
		
	while(1) {
		switch(*msg) {
			case ',':
				*buf = '\0';
				crc ^= ',';
				msg++;
				goto END;
			case '*':
				*buf = '\0';
				msg++;
				goto END;
			case '\0':
				goto END;
			default:
				*buf = *msg;
				crc ^= *msg;
				buf++;
				msg++;
				break;
		}
	}
	
	END:
	return;
}

// $GPGGA,hhmmss.ss,Latitude,N,Longitude,E,FS,NoSV,HDOP,msl,m,Altref,m,DiffAge,DiffStation*cs<CR><LF>
void updateValues(char * data)
{
	char fieldBuf[10];
	getField(data, fieldBuf);
	
	if(strcmp(fieldBuf, GPGGA_header) == 0) {
		
		char dir;
		
		getField(NULL, fieldBuf);
		//calc_utc_time(atof(fieldBuf));
		getField(NULL, fieldBuf);
		GPS_data.lat  = atof(fieldBuf);
		getField(NULL, fieldBuf);
		dir  = fieldBuf[0];
		GPS_data.lat = dm_dd(GPS_data.lat, dir);
		getField(NULL, fieldBuf);
		GPS_data.lon  = atof(fieldBuf);
		getField(NULL, fieldBuf);
		dir  = fieldBuf[0];
		GPS_data.lon = dm_dd(GPS_data.lon, dir);
		getField(NULL, fieldBuf);
		GPS_data.fs      = fieldBuf[0];
		getField(NULL, fieldBuf);
		GPS_data.nosv    = atof(fieldBuf);
		getField(NULL, fieldBuf);
		GPS_data.hdop    = atof(fieldBuf);
		getField(NULL, fieldBuf);
		GPS_data.msl     = atof(fieldBuf);
		getField(NULL, fieldBuf);
		GPS_data.umsl    = fieldBuf[0];
		getField(NULL, fieldBuf);
		GPS_data.altref  = atof(fieldBuf);
		getField(NULL, fieldBuf);
		GPS_data.usep    = fieldBuf[0];
		getField(NULL, fieldBuf);
		GPS_data.diffage = atof(fieldBuf);
		getField(NULL, fieldBuf);
		GPS_data.diffstation = atoi(fieldBuf);
		getField(NULL, fieldBuf);
		int current_crc = strtol(fieldBuf, NULL, 16);
		if(current_crc != crc) {
			//error
		}
	}
	
	/*
	else if (findSentence(GPGLL_header))
	{
		GPS_data.lat_dm     = atof(data[1]);
		GPS_data.latDir    = data[2][0];
		GPS_data.lat_dd    = dm_dd(lat_dm, latDir);
		GPS_data.lon_dm    = atof(data[3]);
		GPS_data.lonDir    = data[4][0];
		GPS_data.lon_dd    = dm_dd(lon_dm, lonDir);

		calc_utc_time(atof(data[5]));

		GPS_data.navStatus = data[6][0];
	}
	else if (findSentence(GPGLV_header))
	{
		// TODO
	}
	else if (findSentence(GPGSA_header))
	{
		// TODO
	}
	else if (findSentence(GPRMC_header))
	{
		calc_utc_time(atof(data[1]));

		GPS_data.navStatus = data[2][0];
		GPS_data.lat_dm    = atof(data[3]);
		GPS_data.latDir    = data[4][0];
		GPS_data.lat_dd    = dm_dd(lat_dm, latDir);
		GPS_data.lon_dm    = atof(data[5]);
		GPS_data.lonDir    = data[6][0];
		GPS_data.lon_dd    = dm_dd(lon_dm, lonDir);
		GPS_data.sog_knots = atof(data[7]);
		GPS_data.cog_true  = atof(data[8]);

		calc_utc_date(atoi(data[9]));
	}
	else if (findSentence(GPVTG_header))
	{
		// TODO
	}*/
}

void setSentence(char * configPacket, char NMEA_num, uint8_t enable)
{
	memcpy(configPacket, CFG_MSG, NMEA_LEN);

	if (enable)
		configPacket[SERIAL_1_POS] = 1;

	configPacket[NMEA_ID_POS] = NMEA_num;
	insertChecksum(configPacket, NMEA_LEN);
}
/*
void NEO_M6_setSentence(nmea_conf_package * configPacket, uint8_t sentence)
{
	*configPacket = defultConfigPacket;
	configPacket->payload[SERIAL_1_POS] = 1;
	configPacket->payload[NMEA_ID_POS] = sentence;
	insertChecksum(configPacket->payload, NMEA_LEN);
}*/
