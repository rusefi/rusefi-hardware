/**
 * @file main.cpp
 * @file Unit tests of SENT
 *
 * @author Andrey Belomutskiy, (c) 2012-2022
 */


#include <stdlib.h>
#include "logicdata_csv_reader.h"

bool hasInitGtest = false;

int main(int argc, char **argv) {
	hasInitGtest = true;


	printf("Hello SENT tests\r\n");

	{	
		CsvReader r;
		r.open("../SENT-recordings/SENT-ETB.csv");

		while (r.haveMore()) {
		    r.processLine(nullptr);
		}
	}


	int result = 0;
	// windows ERRORLEVEL in Jenkins batch file seems to want negative value to detect failure
	return result == 0 ? 0 : -1;
}
