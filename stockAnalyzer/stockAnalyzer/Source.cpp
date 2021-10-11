#include <iostream>
#include <vector>
#include <ctime>
#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <fstream>
#include <limits>
#include <string>
using namespace std;

#include "csv.h"
#include "ImageBMP.h"

class Equity {
public:
	unsigned int d;
	string industry;
	unsigned long long marketCap;
	unsigned int lengthOfDays;
	string ticker;
	vector<string> date;
	vector<float> open;
	vector<float> high;
	vector<float> low;
	vector<float> close;
	vector<unsigned int> volume;

	Equity() {
		industry = "NA";
		marketCap = 0;
		d = 0;
	}
};

class CandleStickPattern {
public:
	vector<string> tickerAndDate;
	string candleStickType;
	string candleStickPattern;
	unsigned int numberOfOccurences;
	float profitabilityOfGivenDay;
	float measuredProfit;
	float greenDay;
	float redDay;
	//float ratioGR;

	CandleStickPattern() {
		candleStickType = "";
		candleStickPattern = "";
		numberOfOccurences = 0;
		profitabilityOfGivenDay = 0.0;
		measuredProfit = 0.0;
		greenDay = redDay = 0.0;// ratioGR = 0.0;
	}

	bool operator < (const CandleStickPattern& pattern) const
	{
		return (measuredProfit >= pattern.measuredProfit);
	}
};

vector<string> preTicker;
vector<unsigned long long> currentMarketCap;
vector<string> tickerIndustry;

vector<Equity> equities;
vector<Equity> stocks;
vector<Equity> etfs;
unsigned int stockLoadIndex = 0;
unsigned int etfLoadIndex = 0;

unsigned int loadXEquities = 0;

//unsigned int N = 5;
//vector<vector<vector<vector<vector<vector<vector<vector<vector<vector<vector<CandleStickPattern>>>>>>>>>>> candles;

void loadEODLatestData(string folderPath) {
	cout << "Loading Historical Equity Data from latest EOD..." << endl;

	string directory = folderPath;

	_finddata_t info;
	intptr_t hFile;

	folderPath = directory + "*.*";
	const char* fp = folderPath.c_str();

	if ((hFile = _findfirst(fp, &info)) != -1L) {

		_findnext(hFile, &info); //read "." hidden directory file

		while (_findnext(hFile, &info) == 0) {

			string fileName = info.name;

			io::CSVReader<7> in(directory + info.name);

			std::string ticker, date = "";
			float open, high, low, close;
			unsigned int volume;
			while (in.read_row(ticker, date, open, high, low, close, volume)) {

				if (std::find(preTicker.begin(), preTicker.end(), ticker) != preTicker.end())
				{
					for (unsigned int e = 0; e < equities.size(); e++) {
						if (equities[e].ticker == ticker) {
							date.insert(4, "-");
							date.insert(7, "-");

							equities[e].date.push_back(date);
							equities[e].close.push_back(close);
							equities[e].open.push_back(open);
							equities[e].low.push_back(low);
							equities[e].high.push_back(high);
							equities[e].volume.push_back(volume);
						}
					}
				}

			}

			//break;
		}
		_findclose(hFile);
	}
}

void loadEquityFolder(string folderPath, string equityType, string specificEquity) {
	cout << "Loading Historical Equity Data..." << endl;

	string directory = folderPath;

	io::CSVReader<3> inCapAndIndustry("C:\\Github\\stockbacktester-master\\tickerData\\tickerCapIndustry.csv");
	try {
		inCapAndIndustry.read_header(io::ignore_extra_column, "Ticker", "Market Cap", "Industry");
	}
	catch (...) {
		cout << "Error reading C:\\Users\\benlo\\Documents\\Stock Data\\data\\tickerData\\tickerCapIndustry.csv" << endl;
	}

	std::string tickerName;
	std:string i_marketCap;
	std::string industryString;
	while (inCapAndIndustry.read_row(tickerName, i_marketCap, industryString)) {
		preTicker.push_back(tickerName);
		if (i_marketCap != "") {
			currentMarketCap.push_back(stoull(i_marketCap));
		}
		else {
			currentMarketCap.push_back(0);
		}
		
		tickerIndustry.push_back(industryString);
	}

	_finddata_t info;
	intptr_t hFile;

	folderPath = directory + "*.*";
	const char* fp = folderPath.c_str();

	if ((hFile = _findfirst(fp, &info)) != -1L) {

		_findnext(hFile, &info); //read "." hidden directory file

		//int stop = 0;

		while (_findnext(hFile, &info) == 0) {
			//stop++;
			//if (stop > 10) {
			//	_findclose(hFile);
			//	return;
			//}

			if (specificEquity != "") {
				if (info.name != specificEquity) {
					continue;
				}
			}

			//loadXEquities++;
			//if (loadXEquities > 500) {
			//	break;
			//}

			if (equityType == "etf") {
				etfs.push_back(Equity());
			}
			else {
				stocks.push_back(Equity());
			}

			string fileName = info.name;
			string tickerLower = fileName.substr(0, fileName.find('.'));
			string tickerUpper = tickerLower;

			std::transform(tickerUpper.begin(), tickerUpper.end(), tickerUpper.begin(), ::toupper);
			if (equityType == "etf") {
				etfs[etfLoadIndex].ticker = tickerUpper;
			}
			else {
				stocks[stockLoadIndex].ticker = tickerUpper;
			}

			ifstream lines;
			string emptyString;
			unsigned int lineCount = 0;
			lines.open(directory + tickerLower + ".us.txt");

			while (!lines.eof()) {
				getline(lines, emptyString);
				lineCount++;
			}

			if (equityType == "etf") {
				etfs[etfs.size() - 1].lengthOfDays = lineCount;
			}
			else {
				stocks[stocks.size() - 1].lengthOfDays = lineCount;
				stocks[stockLoadIndex].close.reserve(lineCount);
				stocks[stockLoadIndex].open.reserve(lineCount);
				stocks[stockLoadIndex].high.reserve(lineCount);
				stocks[stockLoadIndex].low.reserve(lineCount);
				stocks[stockLoadIndex].date.reserve(lineCount);
				stocks[stockLoadIndex].volume.reserve(lineCount);
			}

			io::CSVReader<7> in(directory + tickerLower + ".us.txt");
			bool errorReading = false;
			try {
				in.read_header(io::ignore_extra_column, "Date", "Open", "High", "Low", "Close", "Volume", "OpenInt");
			}
			catch (...) {
				cout << "Error reading " + directory + tickerLower + ".us.txt" << endl;
				errorReading = true;
			}

			int i = 0;
			for (; i < preTicker.size(); i++) {
				if (preTicker[i] == tickerUpper) {
					break;
				}
			}

			if (i < preTicker.size()) {
				if (equityType == "etf") {
					etfs[etfLoadIndex].marketCap = currentMarketCap[i];
					etfs[etfLoadIndex].industry = tickerIndustry[i];
				}
				else {
					stocks[stockLoadIndex].marketCap = currentMarketCap[i];
					stocks[stockLoadIndex].industry = tickerIndustry[i];
				}
			}

			std::string date;
			float open, high, low, close, openint;
			unsigned int volume;
			if (equityType == "etf") {
				while (in.read_row(date, open, high, low, close, volume, openint)) {
					etfs[etfLoadIndex].date.push_back(date);
					etfs[etfLoadIndex].open.push_back(open);
					etfs[etfLoadIndex].high.push_back(high);
					etfs[etfLoadIndex].low.push_back(low);
					etfs[etfLoadIndex].close.push_back(close);
					etfs[etfLoadIndex].volume.push_back(volume);
				}
			}
			else {
				while (in.read_row(date, open, high, low, close, volume, openint)) {
					stocks[stockLoadIndex].date.push_back(date);
					stocks[stockLoadIndex].open.push_back(open);
					stocks[stockLoadIndex].high.push_back(high);
					stocks[stockLoadIndex].low.push_back(low);
					stocks[stockLoadIndex].close.push_back(close);
					stocks[stockLoadIndex].volume.push_back(volume);
				}
			}

			if (equityType == "etf") {
				if (errorReading) {
					etfs.pop_back();
					etfLoadIndex--;
				}
				etfLoadIndex++;
			}
			else {
				if (errorReading) {
					stocks.pop_back();
					stockLoadIndex--;
				}
				stockLoadIndex++;
			}
		}
		_findclose(hFile);
	}
}

bool isOneWhiteSoldier(float o0, float c0, float o1, float c1) {
	if (c0 < o0) { //if previous day was bearish
		if (c1 > o1) { //if current day was bullish
			if (o1 > c0 && o1 < o0) { //if open of current day is greater than close of  previous
				if (c1 > o0) { //if close of first day is greater than open of previous
					return true;
				}
			}
		}
	}

	return false;
}

bool isBullishEngulfing(float o0, float c0, float o1, float c1) {
	if (c0 < o0) { //if previous day was bearish
		if (c1 > o1) { //if current day was bullish
			if (o1 < c0) { //if open of current day is less than close of  previous
				if (c1 > o0) { //if close of first day is greater than open of previous
					return true;
				}
			}
		}
	}

	return false;
}

bool isBullishHarami(float o0, float c0, float o1, float c1) {
	if (c0 < o0) { //if previous day was bearish
		if (c1 > o1) { //if current day was bullish
			if (o1 > c0) { //if open of current day is greater than close of  previous
				if (c1 < o0) { //if close of first day is less than open of previous
					return true;
				}
			}
		}
	}

	return false;
}

bool isBullishThreeLineStrike(float o0, float c0, float o1, float c1, float o2, float c2, float o3, float c3) {
	if (c0 < o0) { //if day1 was bearish
		if (c1 < o1 && c1 < c0) { //if day2 was bearish
			if (c2 < o2 && c2 < c1) { //if day3 was bearish
				if (c3 > o3) { //if day 4 was bullish
					if (c3 > o0 && o3 < c2) { // if day 4 was bullish engulfing of previous three bearish candles
						return true;
					}
				}
			}
		}
	}

	return false;
}

bool isDojiNeutral(float o, float c, float h, float l) {
	if (abs(o - c) < h * 0.001) {
		if (o > (0.33 * (h - l)) + l && o < (0.66 * (h - l)) + l) {
			if (c > (0.33 * (h - l)) + l && c < (0.66 * (h - l)) + l) {
				return true;
			}
		}
	}

	return false;
}

bool isBullishAbandonedBaby(float o0, float c0, float o1, float c1) {
	//if the difference between open and close is less than 0.0001 of stock price then it is a doji 
	//doji classifications
	//neutral, gravestone, dragonfly



	if (c0 < o0) { //if previous day was bearish
		if (c1 > o1) { //if current day was bullish
			if (o1 > c0) { //if open of current day is greater than close of  previous
				if (c1 < o0) { //if close of first day is less than open of previous
					return true;
				}
			}
		}
	}

	return false;
}

float EMA(vector<float> arr) {
	float k = 2.0 / (arr.size());
	
	float emaYesterday = arr[0];
	float emaToday = 0.0;

	for (int e = 1; e < arr.size(); e++) {
		emaToday = (arr[e] * k) + (emaYesterday * (1.0 - k));
		emaYesterday = emaToday;
	}

	return emaToday;
}

float SMA(vector<float> arr) {
	float total = 0.0;

	for (int i = 1; i < arr.size(); i++) {
		total += arr[i];
	}

	return total / (arr.size() - 1);
}

vector<CandleStickPattern> calculateHighestProfitability(unsigned int historyLengthOfCandles, unsigned int givenDay, bool profitUpToGivenDay, unsigned int howManyTopPatterns, string typeOfPattern, unsigned int dividers, string endDate) {
	cout << "Analyzing Equities with pattern division: " << to_string(dividers) << endl;
	
	//givenDay = 0 = gap up/down percentage
	//giveDay > 0 = profit / loss percentage of given day
	vector<CandleStickPattern> topPatterns;
	vector<CandleStickPattern> foundPatterns;

	unsigned int lowestNumberOfOccurences = UINT_MAX;
	unsigned int highestNumberOfOccurences = 0;
	float highestProfitabilityOfGivenDay = 0.0;

	for (unsigned int i = 0; i < equities.size(); i++) {
		if (equities[i].close.at(equities[i].close.size() - 1) <= 1.00)
			continue;
		
		if (equities[i].volume.size() <= givenDay)
			continue;

		//cout << "Analyzing bullish on: " + equities[i].ticker << endl;// +" on date:" + dateOfBullishEngulfing << endl;
		//cout << "FoundPattern size: " + to_string(foundPatterns.size()) << endl;
		/*
		vector<float> SMA;
		vector<float> rsiUpDay;
		vector<float> rsiDownDay;
		vector<float> rsiSMA;
		vector<float> rsiEMA;
		*/
		for (int d = historyLengthOfCandles; d < equities[i].volume.size() - 1 - givenDay; d++) {
			//string dateOfBullishEngulfing = equities[i].date[d];
			
			if (d < historyLengthOfCandles)
				continue;

			if (equities[i].date[d] == "2018-05-07") {
				d += 3;
				continue;
			}

			if (equities[i].date[d] == endDate) {
				break;
			}

			/*
			if (d == 14) {

				float total = 0.0;
				for (int s = 0; s < 14; s++) {
					total += equities[i].close[s];
				}
				total /= 14.0;

				SMA.push_back(total);
				
			}

			if (d > 14) {
				//float total = 0.0;
				//for (int s = d - 14; s < d; s++) {
				//	total += equities[i].close[s];
				//}
				//total /= 14.0;

				SMA.push_back((SMA[SMA.size() - 1] * 13.0 + equities[i].close[d]) / 14.0);

				if (SMA.size() > 14) {
					SMA.erase(SMA.begin());
				}
			}*/

			//rsi SMA
			/*
			if (d == 14) {

				for (int s = 1; s < 15; s++) {
					if (equities[i].close[s] > equities[i].close[s - 1]) {
						rsiUpDay.push_back(equities[i].close[s] - equities[i].close[s - 1]);
						rsiDownDay.push_back(0.0);
					}
					else if (equities[i].close[s - 1] > equities[i].close[s]) {
						rsiUpDay.push_back(0.0);
						rsiDownDay.push_back(equities[i].close[s - 1] - equities[i].close[s]);
					}
					else {
						rsiUpDay.push_back(0.0);
						rsiDownDay.push_back(0.0);
					}
				}
				
				float averageRsiUpDay = 0.0;
				float averageRsiDownDay = 0.0;
				for (int total = 0; total < rsiUpDay.size(); total++) {
					averageRsiUpDay += rsiUpDay[total];
					averageRsiDownDay += rsiDownDay[total];
				}

				averageRsiUpDay /= 14.0;
				averageRsiDownDay /= 14.0;

				rsiSMA.push_back(100.0 - (100.0 / (1.0 + (averageRsiUpDay / averageRsiDownDay))));

			}

			if (d > 14) {

				float averageRsiUpDay = 0.0;
				float averageRsiDownDay = 0.0;
				for (int total = 0; total < rsiUpDay.size(); total++) {
					averageRsiUpDay += rsiUpDay[total];
					averageRsiDownDay += rsiDownDay[total];
				}

				averageRsiUpDay /= 14.0;
				averageRsiDownDay /= 14.0;

				float newRsiUpDayValue = 0.0;
				float newRsiDownDayValue = 0.0;

				if (equities[i].close[d] > equities[i].close[d - 1]) {
					//newRsiUpDayValue = ((averageRsiUpDay * 13.0) + (equities[i].close[d] - equities[i].close[d - 1])) / 14.0;
					//newRsiDownDayValue = ((averageRsiDownDay * 13.0) + (0.0)) / 14.0;

					//rsiUpDay.push_back(newRsiUpDayValue);
					//rsiDownDay.push_back(newRsiDownDayValue);
					newRsiUpDayValue = equities[i].close[d] - equities[i].close[d - 1];
					newRsiDownDayValue = 0.0;

					rsiUpDay.push_back(newRsiUpDayValue);
					rsiDownDay.push_back(newRsiDownDayValue);
				}
				else if (equities[i].close[d - 1] > equities[i].close[d]) {
					//newRsiDownDayValue = ((averageRsiDownDay * 13.0) + (equities[i].close[d - 1] - equities[i].close[d])) / 14.0;
					//newRsiUpDayValue = ((averageRsiUpDay * 13.0) + (0.0)) / 14.0;

					//rsiUpDay.push_back(newRsiDownDayValue);
					//rsiDownDay.push_back(newRsiDownDayValue);

					newRsiDownDayValue = equities[i].close[d - 1] - equities[i].close[d];
					newRsiUpDayValue = 0.0;

					rsiUpDay.push_back(newRsiUpDayValue);
					rsiDownDay.push_back(newRsiDownDayValue);
				}
				else {
					//newRsiUpDayValue = ((averageRsiUpDay * 13.0) + (0.0)) / 14.0;
					//newRsiDownDayValue = ((averageRsiDownDay * 13.0) + (0.0)) / 14.0;

					newRsiUpDayValue = 0.0;
					newRsiDownDayValue = 0.0;

					rsiUpDay.push_back(newRsiUpDayValue);
					rsiDownDay.push_back(newRsiDownDayValue);
				}

				if (rsiUpDay.size() > 14)
					rsiUpDay.erase(rsiUpDay.begin());
				if (rsiDownDay.size() > 14)
					rsiDownDay.erase(rsiDownDay.begin());

				averageRsiUpDay = 0.0;
				averageRsiDownDay = 0.0;
				for (int total = 0; total < rsiUpDay.size(); total++) {
					averageRsiUpDay += rsiUpDay[total];
					averageRsiDownDay += rsiDownDay[total];
				}

				averageRsiUpDay /= 14.0;
				averageRsiDownDay /= 14.0;

				float newValue = 100.0 - (100.0 / (1.0 + (averageRsiUpDay / averageRsiDownDay)));

				rsiSMA.push_back(newValue);

				if (rsiSMA.size() > 14) {
					rsiSMA.erase(rsiSMA.begin());
				}
			}
			

			//rsi EMA
			/*
			if (d == 14) {

				for (int s = 1; s < 15; s++) {
					if (equities[i].close[s] > equities[i].close[s - 1]) {
						rsiUpDay.push_back(equities[i].close[s] - equities[i].close[s - 1]);
						rsiDownDay.push_back(0.0);
					}
					else if (equities[i].close[s - 1] > equities[i].close[s]) {
						rsiUpDay.push_back(0.0);
						rsiDownDay.push_back(equities[i].close[s - 1] - equities[i].close[s]);
					}
					else {
						rsiUpDay.push_back(0.0);
						rsiDownDay.push_back(0.0);
					}
				}

				float averageRsiUpDay = 0.0;
				float averageRsiDownDay = 0.0;
				for (int total = 0; total < rsiUpDay.size(); total++) {
					averageRsiUpDay += rsiUpDay[total];
					averageRsiDownDay += rsiDownDay[total];
				}

				averageRsiUpDay /= 14.0;
				averageRsiDownDay /= 14.0;

				rsiEMA.push_back(100.0 - (100.0 / (1.0 + (averageRsiUpDay / averageRsiDownDay))));

			}

			if (d > 14) {

				float averageRsiUpDay = 0.0;
				float averageRsiDownDay = 0.0;
				for (int total = 0; total < rsiUpDay.size(); total++) {
					averageRsiUpDay += rsiUpDay[total];
					averageRsiDownDay += rsiDownDay[total];
				}

				averageRsiUpDay /= 14.0;
				averageRsiDownDay /= 14.0;

				float newRsiUpDayValue = 0.0;
				float newRsiDownDayValue = 0.0;

				float a = 2.0 / (15.0);

				if (equities[i].close[d] > equities[i].close[d - 1]) {
					newRsiUpDayValue = (( a * (equities[i].close[d] - equities[i].close[d - 1])) + ((1.0 - a) * averageRsiUpDay));
					newRsiDownDayValue = ((1.0 - a) * averageRsiDownDay);

					rsiUpDay.push_back(newRsiUpDayValue);
					rsiDownDay.push_back(newRsiDownDayValue);
				}
				else if (equities[i].close[d - 1] > equities[i].close[d]) {
					newRsiDownDayValue = ((a * (equities[i].close[d - 1] - equities[i].close[d])) + ((1.0 - a) * averageRsiDownDay));
					newRsiUpDayValue = ((1.0 - a) * averageRsiUpDay);

					rsiUpDay.push_back(newRsiDownDayValue);
					rsiDownDay.push_back(newRsiDownDayValue);
				}
				else {
					newRsiUpDayValue = ((1.0 - a) * averageRsiUpDay);
					newRsiDownDayValue = ((1.0 - a) * averageRsiDownDay);

					rsiUpDay.push_back(newRsiUpDayValue);
					rsiDownDay.push_back(newRsiDownDayValue);
				}

				if (rsiUpDay.size() > 14)
					rsiUpDay.erase(rsiUpDay.begin());
				if (rsiDownDay.size() > 14)
					rsiDownDay.erase(rsiDownDay.begin());

				float newValue = 100.0 - (100.0 / (1.0 + (newRsiUpDayValue / newRsiDownDayValue)));

				rsiEMA.push_back(newValue);

				if (rsiEMA.size() > 14) {
					rsiEMA.erase(rsiEMA.begin());
				}
			}*/

			if (typeOfPattern == "bullish engulfing") {

				if (!isBullishEngulfing(equities[i].open[d - 1], equities[i].close[d - 1], equities[i].open[d], equities[i].close[d])) {
					continue;
				}
			}

			if (typeOfPattern == "bullish harami") {
				if (!isBullishHarami(equities[i].open[d - 1], equities[i].close[d - 1], equities[i].open[d], equities[i].close[d])) {
					continue;
				}
			}

			if (typeOfPattern == "one white soldier") {
				if (!isOneWhiteSoldier(equities[i].open[d - 1], equities[i].close[d - 1], equities[i].open[d], equities[i].close[d])) {
					continue;
				}
			}

			//if (typeOfPattern == "abandoned baby") {

			//}

			//if (typeOfPattern == "three line strike") {
			//	if (!isBullishThreeLineStrike(equities[i].open[d - 3], equities[i].close[d - 3], equities[i].open[d - 2], equities[i].close[d - 2], 
			//		equities[i].open[d - 1], equities[i].close[d - 1], equities[i].open[d], equities[i].close[d])) {
			//		continue;
			//	}
			//}
			///if (rsiSMA.size() > 0)
				//if (rsiSMA[rsiSMA.size() - 1] > 70)
					//continue;

			//if (equities[i].volume[d] < equities[i].volume[d - 1])
				//continue;

			//string dateOfBullishEngulfing = equities[i].date[d];
			/*
			float N = 14.0;

			if (d < N + 1)
				continue;

			vector<float> upDays;
			vector<float> downDays;
			vector<float> closingPriceEMA;

			N = N + 1;
			for (int v = d; v > d - N; v--) {
						
				if (equities[i].close[v] > equities[i].close[v - 1]) {
					upDays.push_back(equities[i].close[v] - equities[i].close[v - 1]);
					downDays.push_back(0.0);
				}
				else if(equities[i].close[v - 1] > equities[i].close[v]){
					upDays.push_back(0.0);
					downDays.push_back(equities[i].close[v - 1] - equities[i].close[v]);
				}
				else {
					upDays.push_back(0.0);
					downDays.push_back(0.0);
				}

				closingPriceEMA.push_back(equities[i].close[v]);
			}

			std::reverse(closingPriceEMA.begin(), closingPriceEMA.end());
			std::reverse(upDays.begin(), upDays.end());
			std::reverse(downDays.begin(), downDays.end());

					
					
			float closePriceEma = EMA(closingPriceEMA);

			float emaUpDays = SMA(upDays);
			float emaDownDays = SMA(downDays);

			float rsi = 100.0 - (100.0 / (1.0 + (emaUpDays / emaDownDays)));
			*/

			//int stop = 0;

			CandleStickPattern pattern;

			//parse pattern into string
			string extractedPattern = "";

			float top = 0.0;
			for (int t = historyLengthOfCandles; t >= 0; t--) {
				float temp = max(max(max(equities[i].open[d - t], equities[i].close[d - t]), equities[i].low[d - t]), equities[i].high[d - t]);
				if (temp > top)
					top = temp;
			}
					
			float bottom = FLT_MAX;
			for (int b = historyLengthOfCandles; b >= 0; b--) {
				float temp = min(min(min(equities[i].open[d - b], equities[i].close[d - b]), equities[i].low[d - b]), equities[i].high[d - b]);
				if (temp < bottom)
					bottom = temp;
			}

			float difference = top - bottom;
					
			for (int h = historyLengthOfCandles; h >= 0; h--) {
				//int remainder = (equities[i].low[d - h] - bottom) / (difference / dividers);
				//remainder = (remainder == dividers) ? (dividers - 1) : (remainder);
				//extractedPattern.append(to_string(remainder) + ",");
				//remainder = (equities[i].high[d - h] - bottom) / (difference / dividers);
				//remainder = (remainder == dividers) ? (dividers - 1) : (remainder);
				//extractedPattern.append(to_string(remainder) + ",");
				int remainder = (equities[i].open[d - h] - bottom) / (difference / dividers);
				remainder = (remainder == dividers) ? (dividers - 1) : (remainder);
				extractedPattern.append(to_string(remainder) + "|");
				remainder = (equities[i].close[d - h] - bottom) / (difference / dividers);
				remainder = (remainder == dividers) ? (dividers - 1) : (remainder);
				if (h > 0)
					extractedPattern.append(to_string(remainder) + "|");
				else
					extractedPattern.append(to_string(remainder));
			}


			//"l,h,o,c,l,h,o,c,l,h,o,c,l,h,o,c"
					
			//check if pattern has already been recorded
			unsigned int f = 0;
			for (; f < foundPatterns.size(); f++) {
				if (foundPatterns[f].candleStickPattern == extractedPattern) {
					//if (foundPatterns[f].numberOfOccurences < lowestNumberOfOccurences)
					//	lowestNumberOfOccurences = foundPatterns[f].numberOfOccurences;
					//if (foundPatterns[f].numberOfOccurences > highestNumberOfOccurences)
					//	highestNumberOfOccurences = foundPatterns[f].numberOfOccurences;

					//+1 to pattern number of occurences and add profitability of givenDay
					float addProfitToDate = (equities[i].close[d + givenDay] - equities[i].open[d + givenDay]) / equities[i].open[d + givenDay];
					if (isnan(addProfitToDate))
						addProfitToDate = 0.0;

					foundPatterns[f].tickerAndDate.push_back(equities[i].ticker + ":" + equities[i].date[d] + ":" + to_string(addProfitToDate));
					foundPatterns[f].numberOfOccurences++;
					if (givenDay > 0) {
						if (profitUpToGivenDay == false) {
							//if (((equities[i].close[d + givenDay] - equities[i].open[d + givenDay]) / equities[i].open[d + givenDay]) > highestProfitabilityOfGivenDay)
								//highestProfitabilityOfGivenDay = (equities[i].close[d + givenDay] - equities[i].open[d + givenDay]) / equities[i].open[d + givenDay];

							foundPatterns[f].profitabilityOfGivenDay += (equities[i].close[d + givenDay] - equities[i].open[d + givenDay]) / equities[i].open[d + givenDay];
							if ((equities[i].close[d + givenDay] - equities[i].open[d + givenDay]) / equities[i].open[d + givenDay] > 0.0) {
								foundPatterns[f].greenDay += 1.0;
							}
							else {
								foundPatterns[f].redDay += 1.0;
							}
						}
						//else {
						//	if (((equities[i].close[d + givenDay] - equities[i].open[d + 1]) / equities[i].open[d + 1]) > highestProfitabilityOfGivenDay)
						//		highestProfitabilityOfGivenDay = (equities[i].close[d + givenDay] - equities[i].open[d + 1]) / equities[i].open[d + 1];

						//	foundPatterns[f].profitabilityOfGivenDay += (equities[i].close[d + givenDay] - equities[i].open[d + 1]) / equities[i].open[d + 1];
						//}
					}
					//else {
					//	if (((equities[i].open[d + 1] - equities[i].close[d]) / equities[i].close[d]) > highestProfitabilityOfGivenDay)
					//		highestProfitabilityOfGivenDay = (equities[i].open[d + 1] - equities[i].close[d]) / equities[i].close[d];
								
					//	foundPatterns[f].profitabilityOfGivenDay += (equities[i].open[d + 1] - equities[i].close[d]) / equities[i].close[d]; //calculate profit of gap day
					//}
					break;
				}

			}

			//if did not find pattern add it as new one
					
			if (f >= foundPatterns.size()) {

				//if (foundPatterns[f].numberOfOccurences < lowestNumberOfOccurences)
				//	lowestNumberOfOccurences = foundPatterns[f].numberOfOccurences;
				//if (foundPatterns[f].numberOfOccurences > highestNumberOfOccurences)
				//	highestNumberOfOccurences = foundPatterns[f].numberOfOccurences;

				CandleStickPattern c;

				float addProfitToDate = (equities[i].close[d + givenDay] - equities[i].open[d + givenDay]) / equities[i].open[d + givenDay];
				if (isnan(addProfitToDate))
					addProfitToDate = 0.0;

				c.tickerAndDate.push_back(equities[i].ticker + ":" + equities[i].date[d] + ":" + to_string(addProfitToDate));
				c.candleStickPattern = extractedPattern;
				c.numberOfOccurences = 1;
				if (givenDay > 0) {
					if (profitUpToGivenDay == false) {
						//if (((equities[i].close[d + givenDay] - equities[i].open[d + givenDay]) / equities[i].open[d + givenDay]) > highestProfitabilityOfGivenDay)
							//highestProfitabilityOfGivenDay = (equities[i].close[d + givenDay] - equities[i].open[d + givenDay]) / equities[i].open[d + givenDay];

						c.profitabilityOfGivenDay = (equities[i].close[d + givenDay] - equities[i].open[d + givenDay]) / equities[i].open[d + givenDay];
						if ((equities[i].close[d + givenDay] - equities[i].open[d + givenDay]) / equities[i].open[d + givenDay] > 0.0) {
							c.greenDay += 1.0;
						}
						else {
							c.redDay += 1.0;
						}
					}
					else {
						//if (((equities[i].close[d + givenDay] - equities[i].open[d + 1]) / equities[i].open[d + 1]) > highestProfitabilityOfGivenDay)
							//highestProfitabilityOfGivenDay = (equities[i].close[d + givenDay] - equities[i].open[d + 1]) / equities[i].open[d + 1];

						c.profitabilityOfGivenDay += (equities[i].close[d + givenDay] - equities[i].open[d + 1]) / equities[i].open[d + 1];
					}
				}
				//else {
				//	if (((equities[i].open[d + 1] - equities[i].close[d]) / equities[i].close[d]) > highestProfitabilityOfGivenDay)
				//		highestProfitabilityOfGivenDay = (equities[i].open[d + 1] - equities[i].close[d]) / equities[i].close[d];

				//	c.profitabilityOfGivenDay = (equities[i].open[d + 1] - equities[i].close[d]) / equities[i].close[d]; //calculate profit of gap day
				//}
				foundPatterns.push_back(c);
			}

		}
	}

	//average out the profit and days found of patterns
	for (unsigned int a = 0; a < foundPatterns.size(); a++) {
		foundPatterns[a].profitabilityOfGivenDay = foundPatterns[a].profitabilityOfGivenDay / float(foundPatterns[a].numberOfOccurences);
	}

	//pick top howManyTopPatterns
	//float differenceNumberOfOccurences = 1 + highestNumberOfOccurences - lowestNumberOfOccurences;

	for (unsigned int p = 0; p < foundPatterns.size(); p++) {

		if (foundPatterns[p].profitabilityOfGivenDay <= 0.0)
			continue;


		if (foundPatterns[p].numberOfOccurences == 1) {
			float profit = (foundPatterns[p].profitabilityOfGivenDay) * (foundPatterns[p].greenDay / foundPatterns[p].redDay);
			foundPatterns[p].measuredProfit = profit;
			continue;
		}

		if (foundPatterns[p].numberOfOccurences < 30)
			continue;

		if (foundPatterns[p].greenDay / foundPatterns[p].redDay < 1.0)
			continue;

		float numberOfOccurences = foundPatterns[p].numberOfOccurences;// -lowestNumberOfOccurences;
		if (numberOfOccurences == 0)
			numberOfOccurences = 1;
		//float profit = (numberOfOccurences / differenceNumberOfOccurences) * (foundPatterns[p].profitabilityOfGivenDay / highestProfitabilityOfGivenDay);
		//float profit = numberOfOccurences * foundPatterns[p].profitabilityOfGivenDay;
		//float profit = numberOfOccurences;
		//float profit = foundPatterns[p].profitabilityOfGivenDay;
		float profit = (foundPatterns[p].profitabilityOfGivenDay) * (foundPatterns[p].greenDay / foundPatterns[p].redDay);
		foundPatterns[p].measuredProfit = profit;

		topPatterns.push_back(foundPatterns[p]);

		/*
		if (topPatterns.size() == 0) {
			topPatterns.push_back(foundPatterns[p]);
			continue;
		}

		unsigned int t = 0;
		for (; t < topPatterns.size(); t++) {
			//insert before beginning element if bigger than it
			if (profit > topPatterns[t].measuredProfit && t == 0) {
				topPatterns.insert(topPatterns.begin(), foundPatterns[p]);
				break;
			}

			if (topPatterns.size() == 1) {
				if (profit >= topPatterns[0].measuredProfit) {
					topPatterns.insert(topPatterns.begin(), foundPatterns[p]);
				}
				else {
					topPatterns.push_back(foundPatterns[p]);
				}
				break;
			}

			if (t == topPatterns.size() - 1) {
				if (profit >= topPatterns[t - 1].measuredProfit) {
					topPatterns.insert(topPatterns.begin() + t, foundPatterns[p]);
				}
				else {
					topPatterns.push_back(foundPatterns[p]);
				}
				break;
			}

			if (profit < topPatterns[t].measuredProfit && profit >= topPatterns[t + 1].measuredProfit) {
				topPatterns.insert(topPatterns.begin() + t + 1, foundPatterns[p]);
				break;
			}
		}*/

		std::sort(topPatterns.begin(), topPatterns.end());

		//if (topPatterns.size() > 100)
			//topPatterns.pop_back();


	}

	for (unsigned int t = 0; t < topPatterns.size(); t++) {

		if (typeOfPattern == "bullish engulfing") {
			topPatterns[t].candleStickType = "bullish engulfing";
		}

		if (typeOfPattern == "bullish harami") {
			topPatterns[t].candleStickType = "bullish harami";
		}

		if (typeOfPattern == "one white soldier") {
			topPatterns[t].candleStickType = "one white soldier";
		}
	}

	return topPatterns;
}

float simulation(string startDate, string endDate, float initialDeposit, vector<CandleStickPattern> &candlestickMetric, int tradingMethod) {

	//int s_year = stoi(startDate.substr(0, 4));
	//int s_month = stoi(startDate.substr(5, 7));
	//int s_day = stoi(startDate.substr(9, 11));

	//int e_year = stoi(endDate.substr(0, 4));
	//int e_month = stoi(endDate.substr(5, 7));
	//int e_day = stoi(endDate.substr(9, 11));

	//load in stocks that are actually on the market for the start date
	vector<Equity> activeEquitiesAtStartDate;
	for (unsigned int i = 0; i < equities.size(); i++) {
		for (unsigned int d = 0; d < equities[i].volume.size(); d++) {
			if (equities[i].date[d] == startDate) {
				equities[i].d = d;
				activeEquitiesAtStartDate.push_back(equities[i]);
				break;
			}
		}
	}

	float investment1 = 0.0;
	//float investment2 = 0.0;

	int investment1Day = 0;
	//int investment2Day = 0;

	string dateInvested = "";

	int day = 0;
	float prevMonth = initialDeposit;

	while (true) {
		day++;

		if (day > 30) {
			cout << "Month percentage: " << to_string(((initialDeposit + investment1) / prevMonth) - 1.0) << 
				", total: " << to_string(initialDeposit + investment1) << endl;
			prevMonth = initialDeposit + investment1;
			day = 0;
		}

		CandleStickPattern bestPatternForDay;
		bestPatternForDay.tickerAndDate.push_back("");
		Equity equityToInvestIn;
		equityToInvestIn.industry = "nonefound";
		//check stocks each day for pattern

		//cout << equities[0].date[equities[0].d] << endl;

		for (unsigned int i = 0; i < activeEquitiesAtStartDate.size(); i++) {
			
			if (activeEquitiesAtStartDate[i].d > 2 && activeEquitiesAtStartDate[i].d < activeEquitiesAtStartDate[i].volume.size() - 3) {
				if (activeEquitiesAtStartDate[i].date[activeEquitiesAtStartDate[i].d] == endDate) {
					cout << "removing equity because time reached: " << activeEquitiesAtStartDate[i].ticker << endl;
					activeEquitiesAtStartDate.erase(activeEquitiesAtStartDate.begin() + i);
					continue;
				}

				int d = activeEquitiesAtStartDate[i].d;
				activeEquitiesAtStartDate[i].d++;

				if (!(isBullishEngulfing(activeEquitiesAtStartDate[i].open[d - 1], activeEquitiesAtStartDate[i].close[d - 1], activeEquitiesAtStartDate[i].open[d], activeEquitiesAtStartDate[i].close[d]) ||
					isBullishHarami(activeEquitiesAtStartDate[i].open[d - 1], activeEquitiesAtStartDate[i].close[d - 1], activeEquitiesAtStartDate[i].open[d], activeEquitiesAtStartDate[i].close[d]) ||
					isOneWhiteSoldier(activeEquitiesAtStartDate[i].open[d - 1], activeEquitiesAtStartDate[i].close[d - 1], activeEquitiesAtStartDate[i].open[d], activeEquitiesAtStartDate[i].close[d]))) {
					continue;
				}

				for (int p = 7; p < 12; p++) {
					//parse pattern into string
					string extractedPattern = "";

					float top = 0.0;
					for (int t = 2; t >= 0; t--) {
						float temp = max(max(max(activeEquitiesAtStartDate[i].open[d - t], activeEquitiesAtStartDate[i].close[d - t]), activeEquitiesAtStartDate[i].low[d - t]), activeEquitiesAtStartDate[i].high[d - t]);
						if (temp > top)
							top = temp;
					}

					float bottom = FLT_MAX;
					for (int b = 2; b >= 0; b--) {
						float temp = min(min(min(activeEquitiesAtStartDate[i].open[d - b], activeEquitiesAtStartDate[i].close[d - b]), activeEquitiesAtStartDate[i].low[d - b]), activeEquitiesAtStartDate[i].high[d - b]);
						if (temp < bottom)
							bottom = temp;
					}

					float difference = top - bottom;

					for (int h = 2; h >= 0; h--) {
						int remainder = (activeEquitiesAtStartDate[i].open[d - h] - bottom) / (difference / p);
						remainder = (remainder == p) ? (p - 1) : (remainder);
						extractedPattern.append(to_string(remainder) + "|");
						remainder = (activeEquitiesAtStartDate[i].close[d - h] - bottom) / (difference / p);
						remainder = (remainder == p) ? (p - 1) : (remainder);
						if (h > 0)
							extractedPattern.append(to_string(remainder) + "|");
						else
							extractedPattern.append(to_string(remainder));
					}

					for (unsigned int calibrated = 0; calibrated < candlestickMetric.size(); calibrated++) {
						if (candlestickMetric[calibrated].candleStickPattern == extractedPattern) {
							if (candlestickMetric[calibrated].measuredProfit > bestPatternForDay.measuredProfit) {
								dateInvested = activeEquitiesAtStartDate[i].date[d + 1];
								bestPatternForDay.tickerAndDate[0] = activeEquitiesAtStartDate[i].ticker;
								bestPatternForDay.measuredProfit = candlestickMetric[calibrated].measuredProfit;
								bestPatternForDay.profitabilityOfGivenDay = candlestickMetric[calibrated].profitabilityOfGivenDay;
								bestPatternForDay.greenDay = candlestickMetric[calibrated].greenDay;
								bestPatternForDay.numberOfOccurences = candlestickMetric[calibrated].numberOfOccurences;
								bestPatternForDay.candleStickPattern = candlestickMetric[calibrated].candleStickPattern;
								bestPatternForDay.candleStickType = candlestickMetric[calibrated].candleStickType;
								equityToInvestIn = activeEquitiesAtStartDate[i];
							}
						}
					}
				}
			}
		}

		if (investment1Day >= 1)
			investment1Day++;
		//if (investment2Day >= 1)
		//	investment2Day++;

		if (equityToInvestIn.industry == "nonefound")
			continue;

		if (investment1Day > 2) {
			initialDeposit += investment1;
			investment1 = 0.0;
			investment1Day = 0;
		}
		//if (investment2Day > 2) {
		//	initialDeposit += investment2;
		//	investment2 = 0.0;
		//	investment2Day = 0;
		//}

		//if we have money and found a bestPattern then invest
		if (investment1Day == 0 && bestPatternForDay.numberOfOccurences > 0) {
			investment1 = initialDeposit / 4.0;
			initialDeposit -= investment1;
			float openDay1 = equityToInvestIn.open[equityToInvestIn.d];
			float closeDay1 = equityToInvestIn.close[equityToInvestIn.d];

			float change = (closeDay1 - openDay1) / openDay1;
			cout << "Investment1 for: " << bestPatternForDay.tickerAndDate[0] << ", with: " <<
				to_string(investment1) << ", on: " << dateInvested << ", returned: " << to_string(change) << endl;

			change += 1.0;
			investment1 *= change;
			bestPatternForDay.numberOfOccurences = 0;
			investment1Day++;
			continue;
		}
		/*
		if (investment2Day == 0 && bestPatternForDay.numberOfOccurences > 0) {
			investment2 = initialDeposit / 4.0;
			initialDeposit -= investment2;
			
			float openDay1 = equityToInvestIn.open[equityToInvestIn.d];
			float openDay2 = equityToInvestIn.open[equityToInvestIn.d + 1];

			float change = (openDay2 - openDay1) / openDay1;
			cout << "Investment2 for: " << bestPatternForDay.tickerAndDate[0] << ", with: " << 
				to_string(investment2) << ", on: " << dateInvested << ", returned: " << to_string(change) << endl;

			change += 1.0;
			investment2 *= change;
			bestPatternForDay.numberOfOccurences = 0;
			investment2Day++;
		}*/
	}

	if (investment1 > 0.0)
		initialDeposit += investment1;
	//if (investment2 > 0.0)
	//	initialDeposit += investment2;

	return initialDeposit;
}

const int arrsize = 10;

//{ 5, 10, 15, 20, 25, 30, 35, 40, 45, 50 };
//{ 40, 35, 30, 25, 20, 25, 20, 15, 10, 5 };
//{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
double stable = 0.1;
double percentageInvestedAt[arrsize] = { stable, stable, stable, stable, stable, stable, stable, stable, stable, stable };

//{ 0.05, 0.05, 0.05, 0.05, 0.10, 0.10, 0.10, 0.15, 0.15, 0.20 };
//{ 0.20, 0.20, 0.15, 0.10, 0.10, 0.05, 0.05, 0.05, 0.05, 0.05 };
//{ 0.10, 0.10, 0.10, 0.10, 0.10, 0.10, 0.10, 0.10, 0.10, 0.10 };
double percentageSharesInvested[arrsize] = { 0.10, 0.10, 0.10, 0.10, 0.10, 0.10, 0.10, 0.10, 0.10, 0.10 };

double maxInvestRet = 0;

void backtest() {

	string startDate = "2008-11-06";
	string endDate = "2018-03-28";

	for (unsigned int i = 0; i < equities.size(); i++) {
		for (unsigned int d = 0; d < equities[i].volume.size(); d++) {
			if (equities[i].date[d] == startDate) {
				equities[i].d = d;
				break;
			}
		}
	}

	int spxl = 0;
	int spy = 1;
	int sqqq = 2;

	int selectedEtf = spxl;

	int percentageHasBeenInvested[arrsize] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

	double const start = 1000;

	double startingInvestmentBank = 1000;
	double modifiedBank = 1000;
	double investedSharesInEtf = 0;

	string startDip = startDate;

	double etfPriceHigh = equities[selectedEtf].close[equities[selectedEtf].d - 1];

	for (unsigned int d = equities[selectedEtf].d; equities[selectedEtf].date[d] != endDate; d++) {
		if (equities[selectedEtf].close[d] > etfPriceHigh) {

			if (investedSharesInEtf != 0) {

				startingInvestmentBank = modifiedBank + (investedSharesInEtf * equities[selectedEtf].close[d]);
				modifiedBank = startingInvestmentBank;

				for (int i = 0; i < arrsize; i++)
					percentageHasBeenInvested[i] = 0;

				//cout << "dip from " + startDip + " to " + equities[selectedEtf].date[d] << endl;
				//cout << "share price: " << endl;
				//cout << equities[selectedEtf].close[d] << endl;
				//cout << "total profit: " << endl;
				//cout << modifiedBank << endl;
				//cout << endl;

			}
			investedSharesInEtf = 0;
			etfPriceHigh = equities[selectedEtf].close[d];

			startDip = equities[selectedEtf].date[d];
		}
		else {
			double percentageCurr = 100.0 * (1.0 - (equities[selectedEtf].close[d] / etfPriceHigh));

			for (int i = 0; i < arrsize; i++) {
				if (percentageCurr > percentageInvestedAt[i] && percentageHasBeenInvested[i] == 0) {
					percentageHasBeenInvested[i] = 1;

					investedSharesInEtf += (startingInvestmentBank * percentageSharesInvested[i]) / equities[selectedEtf].close[d];
					modifiedBank -= startingInvestmentBank * percentageSharesInvested[i];
				}
			}

			//if below percentage trigger
			//then invest
		}
	}

	if (startingInvestmentBank > maxInvestRet) {
		cout << "percentageInvestedAt: " << endl;
		cout << percentageInvestedAt[0] << ", " << percentageInvestedAt[1] << ", " << percentageInvestedAt[2] << ", " << percentageInvestedAt[3] << ", " << percentageInvestedAt[4] << ", " <<
			percentageInvestedAt[5] << ", " << percentageInvestedAt[6] << ", " << percentageInvestedAt[7] << ", " << percentageInvestedAt[8] << ", " << percentageInvestedAt[9] << endl;
		maxInvestRet = startingInvestmentBank;
		cout << "total gain: " << endl;
		cout << startingInvestmentBank / start << endl;
		cout << endl;
	}

	//cout << "total gain: " << endl;
	//cout << startingInvestmentBank / start << endl;
}

int main() {

	//create a slider
	// low number of occurrences -------       number of occurrences       ---------- max number

	//rsi will affect G/R ratio: if rsi is 40 or below, on a certain range, is the next day green or red?
	//rsi might affect how much profitability 
	//average volume
	//price within certain range
	//success percentage that profit reached average predicted range
	//percentage your trades were positive

	//when testing, don't buy in if volume < 1,000,000

	string specificEquity = "tqqq.us.txt";
	specificEquity = "";

	//stock
	loadEquityFolder("C:\\Github\\stockbacktester-master\\daily\\test etfs\\", "etf", specificEquity);
	//loadEquityFolder("C:\\Github\\stockbacktester-master\\daily\\us\\nasdaq etfs\\2\\", "etf", specificEquity);
	//loadEquityFolder("C:\\Github\\stockbacktester-master\\daily\\nyse etfs\\", "etf", specificEquity);
	//loadEquityFolder("C:\\Github\\stockbacktester-master\\daily\\us\\nyse etfs\\2\\", "etf", specificEquity);

	//index
	//loadEquityFolder("C:\\Users\\benlo\\Documents\\Stock Data\\data\\daily\\us\\nasdaq etfs\\", "etf", specificEquity);
	//loadEquityFolder("C:\\Users\\benlo\\Documents\\Stock Data\\data\\daily\\us\\nyse etfs\\", "etf", specificEquity);

	equities.insert(equities.end(), stocks.begin(), stocks.end());
	equities.insert(equities.end(), etfs.begin(), etfs.end());

	/*

	string startDate = "2010-02-12";
	string endDate = "2018-03-28";

	for (unsigned int i = 0; i < equities.size(); i++) {
		for (unsigned int d = 0; d < equities[i].volume.size(); d++) {
			if (equities[i].date[d] == startDate) {
				equities[i].d = d;
				break;
			}
		}
	}

	int spxl = 0;
	int spy = 1;
	int tqqq = 2;

	int selectedEtf = tqqq;



	int percentageHasBeenInvested[arrsize] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

	double const start = 1000;

	double startingInvestmentBank = 1000;
	double modifiedBank = 1000;
	double investedSharesInEtf = 0;

	string startDip = startDate;

	double etfPriceHigh = equities[selectedEtf].close[equities[selectedEtf].d - 1];

	for (unsigned int d = equities[selectedEtf].d; equities[selectedEtf].date[d] != endDate; d++) {
		if (equities[selectedEtf].close[d] > etfPriceHigh) {

			if (investedSharesInEtf != 0) {

				startingInvestmentBank = modifiedBank + (investedSharesInEtf * equities[selectedEtf].close[d]);
				modifiedBank = startingInvestmentBank;

				for (int i = 0; i < arrsize; i++)
					percentageHasBeenInvested[i] = 0;

				cout << "dip from " + startDip + " to " + equities[selectedEtf].date[d] << endl;
				cout << "share price: " << endl;
				cout << equities[selectedEtf].close[d] << endl;
				cout << "total profit: " << endl;
				cout << modifiedBank << endl;
				cout << endl;

			}
			investedSharesInEtf = 0;
			etfPriceHigh = equities[selectedEtf].close[d];

			startDip = equities[selectedEtf].date[d];
		}
		else {
			double percentageCurr = 100.0 * (1.0 - (equities[selectedEtf].close[d] / etfPriceHigh));

			for (int i = 0; i < arrsize; i++) {
				if (percentageCurr > percentageInvestedAt[i] && percentageHasBeenInvested[i] == 0) {
					percentageHasBeenInvested[i] = 1;

					investedSharesInEtf += (startingInvestmentBank * percentageSharesInvested[i]) / equities[selectedEtf].close[d];
					modifiedBank -= startingInvestmentBank * percentageSharesInvested[i];
				}
			}

			//if below percentage trigger
			//then invest
		}
	}

	cout << "total gain: " << endl;
	cout << startingInvestmentBank / start << endl;
	

	system("PAUSE");
	return 0;
	*/
	srand(time(NULL));

	int maxVal = 10;

	stable = 0.1;

	for (double i = 0.1; i < 10; i += 0.1) {
		backtest();
		stable = i;
		percentageInvestedAt[0] = stable;
		percentageInvestedAt[1] = stable;
		percentageInvestedAt[2] = stable;
		percentageInvestedAt[3] = stable;
		percentageInvestedAt[4] = stable;
		percentageInvestedAt[5] = stable;
		percentageInvestedAt[6] = stable;
		percentageInvestedAt[7] = stable;
		percentageInvestedAt[8] = stable;
		percentageInvestedAt[9] = stable;
	}

	system("PAUSE");
	return 0;

	for (int a = 1; a < maxVal; a++) {
		cout << "t1" << endl;
		for (int b = 1; b < maxVal; b++)
			for (int c = 1; c < maxVal; c++)
				for (int d = 1; d < maxVal; d++)
					for (int e = 1; e < maxVal; e++) {
						cout << "t2" << endl;
						for (int f = 1; f < maxVal; f++)
							for (int g = 1; g < maxVal; g++)
								for (int h = 1; h < maxVal; h++)
									for (int i = 1; i < maxVal; i++)
										for (int j = 1; j < maxVal; j++) {
											//rand() % 50 + 1;
											percentageInvestedAt[0] = a;
											percentageInvestedAt[1] = b;
											percentageInvestedAt[2] = c;
											percentageInvestedAt[3] = d;
											percentageInvestedAt[4] = e;
											percentageInvestedAt[5] = f;
											percentageInvestedAt[6] = g;
											percentageInvestedAt[7] = h;
											percentageInvestedAt[8] = i;
											percentageInvestedAt[9] = j;

											backtest();
										}
					}
	}

	
	system("PAUSE");
	return 0;
	
	//loadEODLatestData("C:\\Github\\stockbacktester-master\\daily\\eodNYSE\\");
	//loadEODLatestData("C:\\Github\\stockbacktester-master\\daily\\eodNASDAQ\\");

	int averageDays = 0;
	for (int d = 0; d < equities.size(); d++) {
		averageDays += equities[d].volume.size();
	}

	averageDays /= equities.size(); //2092

	cout << "average stock lifetime in days: " << averageDays << endl;
	
	CandleStickPattern bestPatternWithSettings;

	long int numberOfGreenDays = 0;
	double averageLostOnGreenDay = 0.0;

	for (unsigned int i = 0; i < equities.size(); i++) {
		for (unsigned int d = 0; d < equities[i].volume.size(); d++) {
			if (equities[i].close[d] > equities[i].open[d]) {
				numberOfGreenDays++;
				if (!isnan((equities[i].open[d] - equities[i].low[d]) / equities[i].open[d]))
					averageLostOnGreenDay += ((equities[i].open[d] - equities[i].low[d]) / equities[i].open[d]);
			}
		}
	}

	cout << "average lost on a green day: " << to_string(averageLostOnGreenDay / numberOfGreenDays) << endl;

	/*
	int recordI = 1;
	int recordJ = 8;

	for (int i = 1; i < 5; i++) {
		for (int j = 8; j < 10; j++) {
			cout << "days going back: " + to_string(i) + ", divided into: " + to_string(j) << endl;
			vector<CandleStickPattern> top100BullishEngulfing = calculateHighestProfitability(i, 1, false, 100, "bullish engulfing", j);
			if (top100BullishEngulfing.size() > 0) {

				cout << "best candlestick measured profit of this combination: " + to_string(top100BullishEngulfing[0].measuredProfit) << endl;

				if (top100BullishEngulfing[0].measuredProfit > bestPatternWithSettings.measuredProfit) {
					bestPatternWithSettings = top100BullishEngulfing[0];
					recordI = i;
					recordJ = j;
				}
			}
		}
	}

	vector<CandleStickPattern> top100BullishEngulfing = calculateHighestProfitability(2, 1, false, 100, "bullish engulfing", 8);

	cout << "occurences: " + to_string(bestPatternWithSettings.numberOfOccurences) + ", average profit: " +
		to_string(bestPatternWithSettings.profitabilityOfGivenDay) + ", totalProfit: " +
		to_string(bestPatternWithSettings.measuredProfit)
		+ ", G/R ratio: " + to_string(bestPatternWithSettings.greenDay / bestPatternWithSettings.redDay) << endl;

	cout << "days going back: " + to_string(recordI) + ", divided into: " + to_string(recordJ) << endl;
	*/

	
	/*
	vector<CandleStickPattern> topBullishEngulfing7 = calculateHighestProfitability(2, 1, false, 100, "bullish engulfing", 7, "2007-10-08");
	vector<CandleStickPattern> topBullishEngulfing8 = calculateHighestProfitability(2, 1, false, 100, "bullish engulfing", 8, "2007-10-08");
	vector<CandleStickPattern> topBullishEngulfing9 = calculateHighestProfitability(2, 1, false, 100, "bullish engulfing", 9, "2007-10-08");
	vector<CandleStickPattern> topBullishEngulfing10 = calculateHighestProfitability(2, 1, false, 100, "bullish engulfing", 10, "2007-10-08");
	vector<CandleStickPattern> topBullishEngulfing11 = calculateHighestProfitability(2, 1, false, 100, "bullish engulfing", 11, "2007-10-08");

	vector<CandleStickPattern> topBullishHarami7 = calculateHighestProfitability(2, 1, false, 100, "bullish harami", 7, "2007-10-08");
	vector<CandleStickPattern> topBullishHarami8 = calculateHighestProfitability(2, 1, false, 100, "bullish harami", 8, "2007-10-08");
	vector<CandleStickPattern> topBullishHarami9 = calculateHighestProfitability(2, 1, false, 100, "bullish harami", 9, "2007-10-08");
	vector<CandleStickPattern> topBullishHarami10 = calculateHighestProfitability(2, 1, false, 100, "bullish harami", 10, "2007-10-08");
	vector<CandleStickPattern> topBullishHarami11 = calculateHighestProfitability(2, 1, false, 100, "bullish harami", 11, "2007-10-08");

	vector<CandleStickPattern> topBullishOneWhiteSoldier7 = calculateHighestProfitability(2, 1, false, 100, "one white soldier", 7, "2007-10-08");
	vector<CandleStickPattern> topBullishOneWhiteSoldier8 = calculateHighestProfitability(2, 1, false, 100, "one white soldier", 8, "2007-10-08");
	vector<CandleStickPattern> topBullishOneWhiteSoldier9 = calculateHighestProfitability(2, 1, false, 100, "one white soldier", 9, "2007-10-08");
	vector<CandleStickPattern> topBullishOneWhiteSoldier10 = calculateHighestProfitability(2, 1, false, 100, "one white soldier", 10, "2007-10-08");
	vector<CandleStickPattern> topBullishOneWhiteSoldier11 = calculateHighestProfitability(2, 1, false, 100, "one white soldier", 11, "2007-10-08");
	
	vector<CandleStickPattern> finalResults;
	finalResults.insert(finalResults.end(), topBullishEngulfing7.begin(), topBullishEngulfing7.end());
	finalResults.insert(finalResults.end(), topBullishEngulfing8.begin(), topBullishEngulfing8.end());
	finalResults.insert(finalResults.end(), topBullishEngulfing9.begin(), topBullishEngulfing9.end());
	finalResults.insert(finalResults.end(), topBullishEngulfing10.begin(), topBullishEngulfing10.end());
	finalResults.insert(finalResults.end(), topBullishEngulfing11.begin(), topBullishEngulfing11.end());
	finalResults.insert(finalResults.end(), topBullishHarami7.begin(), topBullishHarami7.end());
	finalResults.insert(finalResults.end(), topBullishHarami8.begin(), topBullishHarami8.end());
	finalResults.insert(finalResults.end(), topBullishHarami9.begin(), topBullishHarami9.end());
	finalResults.insert(finalResults.end(), topBullishHarami10.begin(), topBullishHarami10.end());
	finalResults.insert(finalResults.end(), topBullishHarami11.begin(), topBullishHarami11.end());
	finalResults.insert(finalResults.end(), topBullishOneWhiteSoldier7.begin(), topBullishOneWhiteSoldier7.end());
	finalResults.insert(finalResults.end(), topBullishOneWhiteSoldier8.begin(), topBullishOneWhiteSoldier8.end());
	finalResults.insert(finalResults.end(), topBullishOneWhiteSoldier9.begin(), topBullishOneWhiteSoldier9.end());
	finalResults.insert(finalResults.end(), topBullishOneWhiteSoldier10.begin(), topBullishOneWhiteSoldier10.end());
	finalResults.insert(finalResults.end(), topBullishOneWhiteSoldier11.begin(), topBullishOneWhiteSoldier11.end());
	
	cout << "Sorting results..." << endl;
	std::sort(finalResults.begin(), finalResults.end());

	ofstream fileResults;
	fileResults.open("results.txt");
	fileResults << "MP,G|R,AverageProfit,Occurences,Pattern,Type" << endl;
	for (unsigned int r = 0; r < finalResults.size(); r++) {
		fileResults << to_string(finalResults[r].measuredProfit) << "," << to_string(finalResults[r].greenDay / finalResults[r].redDay) << "," <<
		to_string(finalResults[r].profitabilityOfGivenDay) << "," << to_string(finalResults[r].numberOfOccurences) << "," <<
		finalResults[r].candleStickPattern << "," << finalResults[r].candleStickType << endl;
	}

	fileResults.close();
	*/

	/*
	//TESTING
	vector<CandleStickPattern> calibratedCandlePatterns;

	io::CSVReader<6> in("results20071008.txt");
	float mp, gr, ap, occ = 0.0;
	string pattern, type = "";
	in.read_header(io::ignore_extra_column, "MP", "G|R", "AverageProfit", "Occurences", "Pattern", "Type");
	while (in.read_row(mp, gr, ap, occ, pattern, type)) {
		CandleStickPattern c;
		c.candleStickPattern = pattern;
		c.candleStickType = type;
		c.greenDay = gr;
		c.redDay = gr;
		c.measuredProfit = mp;
		c.numberOfOccurences = occ;
		c.profitabilityOfGivenDay = ap;

		calibratedCandlePatterns.push_back(c);
	}
	

	float totalProfit = simulation("2009-05-12", "2010-05-12", 10000.0, calibratedCandlePatterns, 0);
	cout << "Total profit: " << totalProfit << endl;
	

	/*
	for (int i = 0; i < topBullishEngulfing7.size(); i++) {
		cout << topBullishEngulfing7[i].candleStickType << endl;
		cout << "occurences: " + to_string(topBullishEngulfing7[i].numberOfOccurences) + ", average profit: " +
			to_string(topBullishEngulfing7[i].profitabilityOfGivenDay) + ", totalProfit: " +
			to_string(topBullishEngulfing7[i].measuredProfit)
			+ ", G/R ratio: " + to_string(topBullishEngulfing7[i].greenDay / topBullishEngulfing7[i].redDay) << endl;
	}
	*/

	/*
	for (int i = 0; i < topBullishEngulfing7.size(); i++) {
		if (topBullishEngulfing7.size() > 0) {
			for (int j = 0; j < topBullishEngulfing7[i].tickerAndDate.size(); j++) {
				cout << topBullishEngulfing7[i].tickerAndDate[j] << endl;
			}
		}
	}
	*/

	/*
	clock_t begin = clock();
	
	clock_t end = clock();
	double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;

	cout << elapsed_secs << endl;*/
	/*
	int greenRedCount = 0;
	float greenRedTotal = 0;

	string tickerHighest = "";
	float greenOverRed = 0.0;

	for (int i = 0; i < stocks.size(); i++) {

		if (stocks[i].volume.size() < 1825 || stocks[i].volume.size() > 2190) {
			continue;
		}

		//if (stocks[i].ticker != "LPLA") {
		//	continue;
		//}

		if (stocks[i].close.size() == 0) {
			continue;
		}

		if (stocks[i].close[stocks[i].close.size() - 1] < 5)
			continue;

		float green = 0;
		float red = 0;

		for (int d = 1; d < stocks[i].volume.size() - 1; d++) {
			if (stocks[i].close[d - 1] < stocks[i].open[d - 1]) {
				if (stocks[i].close[d] > stocks[i].open[d]) {
					//if (stocks[i].open[d] < stocks[i].close[d - 1]) {
						//if (stocks[i].close[d] > stocks[i].open[d - 1]) {
					if (stocks[i].low[d] < stocks[i].low[d - 1] && stocks[i].open[d] < stocks[i].close[d - 1]) {
						if (stocks[i].high[d] > stocks[i].high[d - 1] && stocks[i].close[d] > stocks[i].open[d - 1]) {
							//cout << stocks[i].date[d] << endl;
							if (stocks[i].close[d + 1] > stocks[i].open[d + 1]) {
								green++;
							}
							else {
								red++;
							}
						}
					}
				}
			}
		}

		if (green / red >= 0.0001 && green / red < 100) {
			//cout << "G/R After Bullish Engulfing is: " + to_string(green / red) << endl;
			cout << "Analyzing Stock: " + stocks[i].ticker + " G/R: " + to_string(green / red) << endl;
			greenRedCount++;
			greenRedTotal += (green / red);

			if (green / red > greenOverRed) {
				tickerHighest = stocks[i].ticker;
				greenOverRed = green / red;
			}
		}
	}

	cout << "Results: " + to_string(greenRedTotal / greenRedCount) << endl;
	cout << "Highest: " + tickerHighest + " with ratio: " + to_string(greenOverRed) << endl;

	//unsigned int totalDays = 0;
	//for (int i = 0; i < stocks.size(); i++) {
	//	totalDays += stocks[i].volume.size();
		//cout << stocks[i].ticker + ", days: " + to_string(stocks[i].volume.size()) << endl;
	//}
	*/
	
	system("PAUSE");
	return 0;
}
