/*
 * @file logicdata_csv_reader.cpp
 *
 * @date Jun 26, 2021
 * @author Andrey Belomutskiy, (c) 2012-2021
 */

#include "logicdata_csv_reader.h"
#include <stdlib.h>
#include <string>
#include <string.h>

static char* trim(char *str) {
	while (str != nullptr && str[0] == ' ') {
		str++;
	}
	return str;
}

CsvReader::~CsvReader() {
	if (fp) {
		fclose(fp);
	}
}

void CsvReader::open(const char *fileName) {
	printf("Reading from %s\r\n", fileName);
	fp = fopen(fileName, "r");
//	ASSERT_TRUE(fp != nullptr);
}

bool CsvReader::haveMore() {
	bool result = fgets(buffer, sizeof(buffer), fp) != nullptr;
	m_lineIndex++;
	if (m_lineIndex == 0) {
		// skip header
		return haveMore();
	}

    if (m_lineIndex == 100) {
        printf("It was enough\r\n");
    	exit(0);
    }

	return result;
}

double CsvReader::readTimestampAndValues(double *v) {
	const char s[2] = ",";
	char *line = buffer;

	char *timeStampstr = trim(strtok(line, s));
	double timeStamp = std::stod(timeStampstr);

	for (size_t i = 0; i < 1; i++) {
		char *triggerToken = trim(strtok(nullptr, s));
		v[i] = std::stod(triggerToken);
	}

	return timeStamp;
}

void CsvReader::processLine(void *arg) {
	const char s[2] = ",";
	char *timeStampstr = trim(strtok(buffer, s));

	bool newState[1];

	for (size_t i = 0;i<1;i++) {
		char * triggerToken = trim(strtok(nullptr, s));
		newState[0] = triggerToken[0] == '1';
	}

	if (timeStampstr == nullptr) {
//		firmwareError(OBD_PCM_Processor_Fault, "End of File");
		return;
	}

	double timeStamp = std::stod(timeStampstr);

	timeStamp += m_timestampOffset;

	printf("timestamp %.11f %d\r\n", timeStamp, newState[0]);
}

void CsvReader::readLine(void *arg) {
	if (!haveMore())
		return;
	processLine(arg);
}
