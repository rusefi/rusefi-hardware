/*
 * @file logicdata_csv_reader.h
 *
 * @date Jun 26, 2021
 * @author Andrey Belomutskiy, (c) 2012-2021
 */
#include <cstdio>

class CsvReader {
public:
	~CsvReader();

	void open(const char *fileName);
	bool haveMore();
	void processLine(void *arg);
	void readLine(void *arg);
	double readTimestampAndValues(double *v);

	int lineIndex() const {
		return m_lineIndex;
	}

private:
	const double m_timestampOffset = 0;

	FILE *fp = nullptr;
	char buffer[255];

	bool currentStatex = 0;

	int m_lineIndex = -1;
};

