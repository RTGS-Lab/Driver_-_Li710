//© 2023 Regents of the University of Minnesota. All rights reserved.

#include <LI710.h>

LI710::LI710(SDI12Talon& talon_, uint8_t talonPort_, uint8_t sensorPort_, uint8_t version): talon(talon_)
{
	//Only update values if they are in range, otherwise stick with default values
	if(talonPort_ > 0) talonPort = talonPort_ - 1;
	else talonPort = 255; //Reset to null default if not in range
	if(sensorPort_ > 0) sensorPort = sensorPort_ - 1;
	else sensorPort = 255; //Reset to null default if not in range 
	sensorInterface = BusType::SDI12; 
}

String LI710::begin(time_t time, bool &criticalFault, bool &fault)
{
	// Serial.println("LI710 - BEGIN"); //DEBUG!
	// presSensor.begin(Wire, 0x76); //DEBUG!
	// if(rhSensor.begin(0x44) == false) {
	// 	Serial.println("\tSHT31 Init Fail"); //DEBUG!
	// 	throwError(SHT3X_INIT_ERROR | talonPortErrorCode); //Error subtype = I2C error
	// } 
	
	// Wire.beginTransmission(0x76);
	// int error = Wire.endTransmission();
	// if(error != 0) {
	// 	Serial.println("\tDPS368 Init Fail"); //DEBUG!
	// 	throwError(DPS368_INIT_ERROR | (error << 12) | talonPortErrorCode); //Error subtype = I2C error
	// }
	
	// Wire.beginTransmission(0x44);
	// int errorB = Wire.endTransmission();
	// ret = pres.measureTempOnce(temperature, oversampling);
	// Serial.print("INIT: ");
	// if(errorA == 0 || errorB == 0) Serial.println("PASS");
	// else {
	// 	Serial.print("ERR - ");
	// 	if(errorA != 0) {
	// 		Serial.print("A\t");
	// 		throwError(SHT3X_I2C_ERROR | (errorA << 12) | talonPortErrorCode); //Error subtype = I2C error
	// 	}
	// 	if(errorB != 0) Serial.print("B\t");
	// 	Serial.println("");
	// }
	return ""; //DEBUG!
}

String LI710::selfDiagnostic(uint8_t diagnosticLevel, time_t time)
{
	if(getSensorPort() == 0) throwError(FIND_FAIL); //If no port found, report failure
	else if(isPresent() == false) throwError(DETECT_FAIL); //If sensor port is good, but fail to detect sensor, throw error 
	String output = "\"LiCor ET\":{";
	if(diagnosticLevel == 0) {
		//TBD
	}

	if(diagnosticLevel <= 1) {
		//TBD
	}

	if(diagnosticLevel <= 2) {
		//TBD
	}

	if(diagnosticLevel <= 3) {
		//TBD
 	}

	if(diagnosticLevel <= 4) {
		//TBD
	}

	if(diagnosticLevel <= 5) {
		if(getSensorPort() != 0 && isPresent() == true) { //Test as normal
			String adr = talon.sendCommand("?!");
			int adrVal = adr.toInt();
			output = output + "\"Adr\":";
			if(adr.equals("") || (!adr.equals("0") && adrVal == 0)) {
				output = output + "null,"; //If no return, report null
				for(int i = 0; i < 8; i++) { //Interate over all, ignoring sequence number and diagnostic value
					output = output + appendData(-9999, group3Labels[i], group3Precision[i]); //Append null values
				}
				output = output + appendData(-9999, "TILT", 0, false); //Ignore trailing comma for last entry
			}
			else {
				output = output + adr + ","; //Otherwise report the read value
				//Grab other diagnostic values based on the address read
				String data2 = talon.continuousMeasurmentCRC(2, adrVal);
				// delay(100);
				String data3 = talon.continuousMeasurmentCRC(3, adrVal);
				// delay(100);

				// talon.testCRC(data1); //DEBUG!
				if(!talon.testCRC(data2) || !talon.testCRC(data3)) {
					// Serial.println("LI710 CRC FAIL");
					// continue; //If ANY CRC is bad, try again
					for(int i = 0; i < 8; i++) { //Interate over all, ignoring sequence number and diagnostic value
						output = output + appendData(-9999, group3Labels[i], group3Precision[i]); //Append null values
					}
					output = output + appendData(-9999, "TILT", 0, false); //Ignore trailing comma for last entry
				}

				else {
					float data2Vals[8] = {0.0};
					float data3Vals[8] = {0.0};

					parseData(data2, data2Vals, 8);  //Parse all data
					parseData(data3, data3Vals, 8);


					for(int i = 0; i < 8; i++) { //Interate over all, ignoring sequence number and diagnostic value
						output = output + appendData(data3Vals[i], group3Labels[i], group3Precision[i]);
						// if(data0Vals[i] == -9999) output = output + "\"" + group0Labels[i] + "\":null,"; //Append null if value is error indicator
						// else output = output + "\"" + group0Labels[i] + "\":" + String(data0Vals[i], group0Precision[i]) + ","; //Otherwise, append as normal using fixed specified precision
					}

					output = output + appendData(data2Vals[7], "TILT", 0, false); //Ignore trailing comma for last entry
				}
			}
			output = output + ",";
		}
		else output = output + "\"Adr\":null,\"PUMP_V\":null,\"PA_CELL\":null,\"RH_CELL\":null,\"TA_CELL\":null,\"RH_ENCL\":null,\"FLOW\":null,\"INPUT_V\":null,\"DATA_QC\":null,"; //Else append null string
		
	}
	return output + "\"Pos\":[" + getTalonPortString() + "," + getSensorPortString() + "]}"; //Write position in logical form - Return compleated closed output
}

String LI710::getMetadata()
{
	uint8_t adr = (talon.sendCommand("?!")).toInt(); //Get address of local device 
	String id = talon.command("I", adr);
	Serial.println(id); //DEBUG!
	String sdi12Version;
	String mfg;
	String model;
	String senseVersion;
	String sn;
	if((id.substring(0, 1)).toInt() != adr) { //If address returned is not the same as the address read, throw error
		Serial.println("ADDRESS MISMATCH!"); //DEBUG!
		//Throw error!
		sdi12Version = "null";
		mfg = "null";
		model = "null";
		senseVersion = "null";
		sn = "null";
	}
	else {
		sdi12Version = (id.substring(1,3)).trim(); //Grab SDI-12 version code
		mfg = (id.substring(3, 11)).trim(); //Grab manufacturer
		model = (id.substring(11,17)).trim(); //Grab sensor model name
		senseVersion = (id.substring(17,20)).trim(); //Grab version number
		sn = (id.substring(20,33)).trim(); //Grab the serial number 
	}
	String metadata = "\"LiCor ET\":{";
	// if(error == 0) metadata = metadata + "\"SN\":\"" + uuid + "\","; //Append UUID only if read correctly, skip otherwise 
	metadata = metadata + "\"Hardware\":\"" + senseVersion + "\","; //Report sensor version pulled from SDI-12 system 
	metadata = metadata + "\"Firmware\":\"" + FIRMWARE_VERSION + "\","; //Static firmware version 
	metadata = metadata + "\"SDI12_Ver\":\"" + sdi12Version.substring(0,1) + "." + sdi12Version.substring(1,2) + "\",";
	metadata = metadata + "\"ADR\":" + String(adr) + ",";
	metadata = metadata + "\"Mfg\":\"" + mfg + "\",";
	metadata = metadata + "\"Model\":\"" + model + "\",";
	metadata = metadata + "\"SN\":\"" + sn + "\",";
	//GET SERIAL NUMBER!!!! //FIX!
	metadata = metadata + "\"Pos\":[" + getTalonPortString() + "," + getSensorPortString() + "]"; //Concatonate position 
	metadata = metadata + "}"; //CLOSE  
	return metadata; 
}

String LI710::getData(time_t time)
{
	String output = "\"LiCor ET\":{"; //OPEN JSON BLOB
	bool readDone = false;
	if(getSensorPort() != 0) { //Check both for detection 
		for(int i = 0; i < talon.retryCount; i++) {
			if(!isPresent()) {
				// Serial.print("LI710 PRESENT FAIL"); //DEBUG!
				continue; //If presence check fails, try again
			}

			int adr = talon.getAddress();
			if(adr < 0) {
				// Serial.print("LI710 ADR FAIL = "); //DEBUG!
				// Serial.println(adr);
				continue; //If address is out of range, try again
			}

			// int waitTime = talon.startMeasurmentCRC(adr);
			// if(waitTime <= 0) {
			// 	// Serial.print("TDR315 Wait Time = "); //DEBUG!
			// 	// Serial.println(waitTime);
			// 	continue; //If wait time out of range, try again
			// }
			// uint8_t adr = (talon.sendCommand("?!")).toInt(); //Get address of local device 
			// String stat = talon.command("MC", adr);

			// Serial.print("STAT: "); //DEBUG!
			// Serial.println(stat);

			
			talon.command("XT", adr); //Start next round of conversion 
			// delay(waitTime*1000 + 500); //Wait for number of seconds requested, plus half a second to make sure
			String data0 = talon.continuousMeasurmentCRC(0, adr);
			delay(100);
			String data1 = talon.continuousMeasurmentCRC(1, adr);
			delay(100);
			String data2 = talon.continuousMeasurmentCRC(2, adr);
			delay(100);

			// talon.testCRC(data1); //DEBUG!
			if(!talon.testCRC(data0) || !talon.testCRC(data1) || !talon.testCRC(data2)) {
				// Serial.println("LI710 CRC FAIL");
				continue; //If ANY CRC is bad, try again
			}

			float data0Vals[9] = {0.0};
			float data1Vals[9] = {0.0};
			float data2Vals[8] = {0.0};

			parseData(data0, data0Vals, 9);  //Parse all data
			parseData(data1, data1Vals, 9);
			parseData(data2, data2Vals, 8);

			decodeDiag(data0Vals[8]); //Parse and report error values 
			decodeDiag(data1Vals[8]);

			//////////////// REPORT DIAG VALS!!!!!!!! /////////////


			for(int i = 0; i < 7; i++) { //Interate over all, ignoring sequence number and diagnostic value
				output = output + appendData(data0Vals[i], group0Labels[i], group0Precision[i]);
				// if(data0Vals[i] == -9999) output = output + "\"" + group0Labels[i] + "\":null,"; //Append null if value is error indicator
				// else output = output + "\"" + group0Labels[i] + "\":" + String(data0Vals[i], group0Precision[i]) + ","; //Otherwise, append as normal using fixed specified precision
			}

			output = output + appendData(data1Vals[7], "SAMP_CNT", 0);
			output = output + appendData(data2Vals[0], "AH", 2);
			output = output + appendData(data2Vals[2], "SVP", 2);
			output = output + appendData(data2Vals[7], "TD", 2, false); //Ignore trailing comma for last entry

			// float sensorData[9] = {0.0}; //Store the 9 vals from the sensor in float form
			// if((data.substring(0, data.indexOf("+"))).toInt() != adr) { //If address returned is not the same as the address read, throw error
			// 	Serial.println("ADDRESS MISMATCH!"); //DEBUG!
			// 	throwError(talon.SDI12_SENSOR_MISMATCH | 0x100 | talonPortErrorCode | sensorPortErrorCode); //Throw error on address change, this is a weird place for this error to happen, but could
			// 	continue; //Try again
			// 	//Throw error!
			// }
			// data.remove(0, 1); //Delete address from start of string
			// for(int i = 0; i < 9; i++) { //Parse string into floats -- do this to run tests on the numbers themselves and make sure formatting is clean
			// 	if(indexOfSep(data) == 0 && indexOfSep(data.substring(indexOfSep(data) + 1)) > 0) { //If string starts with seperator AND this is not the last seperator 
			// 		sensorData[i] = (data.substring(0, indexOfSep(data.substring(indexOfSep(data) + 1)) + 1)).toFloat(); //Extract float from between next two seperators
			// 		Serial.println(data.substring(0, indexOfSep(data.substring(indexOfSep(data) + 1)) + 1)); //DEBUG!
			// 		data.remove(0, indexOfSep(data.substring(indexOfSep(data) + 1)) + 1); //Delete leading entry
			// 	}
			// 	// if(indexOfSep(data) == 0) {
			// 	// 	sensorData[i] = (data.substring(0, indexOfSep(data))).toFloat(); //Extract float from between next two seperators
			// 	// 	Serial.println(data.substring(0, indexOfSep(data))); //DEBUG!
			// 	// 	data.remove(0, indexOfSep(data) + 1); //Delete leading entry
			// 	// }
			// 	else {
			// 		data.trim(); //Trim off trailing characters
			// 		sensorData[i] = data.toFloat();
			// 	}
			// }

			// for(int i = 0; i < 3; i++) { //Parse string into floats -- do this to run tests on the numbers themselves and make sure formatting is clean
				// if(data.indexOf("+") > 0) {
				// 	sensorData[i] = (data.substring(0, data.indexOf("+"))).toFloat();
				// 	Serial.println(data.substring(0, data.indexOf("+"))); //DEBUG!
				// 	data.remove(0, data.indexOf("+") + 1); //Delete leading entry
				// }
				// else {
				// 	data.trim(); //Trim off trailing characters
				// 	sensorData[i] = data.toFloat();
				// }
			// }
			// output = output + "\"VWC\":" + String(sensorData[0]) + ",\"Temperature\":" + String(sensorData[1]) + ",\"Permitivity\":" + String(sensorData[2]) + ",\"EC_BULK\":" + String(sensorData[3]) + ",\"EC_PORE\":" + String(sensorData[4]); //Concatonate data
			readDone = true; //Set flag
			break; //Stop retry
		}	
		if(readDone == false) throwError(talon.SDI12_READ_FAIL | talonPortErrorCode | sensorPortErrorCode); //Only throw read fail error if sensor SHOULD be detected 
	}
	else throwError(FIND_FAIL);
	
	if(getSensorPort() == 0 || readDone == false) output = output + "\"ET\":null,\"LE\":null,\"H\":null,\"VPD\":null,\"PA\":null,\"TA\":null,\"RH\":null,\"SAMP_CNT\":null,\"AH\":null,\"SVP\":null,\"TD\":null"; //Append nulls if no sensor port found, or read did not work
	output = output + ",\"Pos\":[" + getTalonPortString() + "," + getSensorPortString() + "]"; //Concatonate position 
	output = output + "}"; //CLOSE JSON BLOB
	Serial.println(output); //DEBUG!
	return output;
}

bool LI710::isPresent() 
{ //FIX!
	// Wire.beginTransmission(0x77);
	// int errorA = Wire.endTransmission();
	// Wire.beginTransmission(0x76);
	// int errorB = Wire.endTransmission();
	// Serial.print("LI710 TEST: "); //DEBUG!
	// Serial.print(errorA);
	// Serial.print("\t");
	// Serial.println(errorB);
	uint8_t adr = (talon.sendCommand("?!")).toInt();
	
	String id = talon.command("I", adr);
	id.remove(0, 1); //Trim address character from start
	// Serial.print("SDI12 Address: "); //DEBUG!
	// Serial.print(adr);
	// Serial.print(",");
	// Serial.println(id);
	if(id.indexOf("LI-710") > 0) return true; //FIX! Check version here!
	// if(errorA == 0 || errorB == 0) return true;
	else return false;
}

int LI710::indexOfSep(String input)
{
	int pos1 = input.indexOf('+');
	int pos2 = input.indexOf('-');
	if(pos1 >= 0 && pos2 >= 0) return min(pos1, pos2); //If both are positive, just return the straight min
	else return max(pos1, pos2); //If one of them is -1, then return the other one. If both are -1, then you should return -1 anyway
}

String LI710::appendData(float data, String label, uint8_t precision, bool appendComma)
{
	String val = "";
	if(data == -9999 || data == 9999999) val = "\"" + label + "\":null"; //Append null if value is error indicator
	else val = "\"" + label + "\":" + String(data, precision); //Otherwise, append as normal using fixed specified precision

	if(appendComma) return val + ",";
	else return val;
}

bool LI710::parseData(String input, float dataReturn[], uint8_t dataLen)
{
	const uint8_t strLen = input.length(); //Get length of string to make char array
	char inputArr[strLen] = {0}; //Initialize to zero
	input.toCharArray(inputArr, strLen); //Copy to array

	uint8_t numSeps = 0; //Keep track of number of seperators found
	for(int i = 0; i < strLen; i++){
		if(inputArr[i] == '+' or inputArr[i] == '-') numSeps += 1; //Increment seperator count if either +/- is found (Note: CRC vals to not contain + or -)
	}
	if(numSeps != dataLen) {
		throwError(talon.SDI12_SENSOR_MISMATCH | 0x300 | talonPortErrorCode | sensorPortErrorCode); //Throw an error to indicate mismatch in number of reports
		return false; //Return error if number of seperators does not match the requested number of values
	}

	// float sensorData[9] = {0.0}; //Store the 9 vals from the sensor in float form
	
	input.remove(0, 1); //Delete address from start of string
	for(int i = 0; i < dataLen; i++) { //Parse string into floats -- do this to run tests on the numbers themselves and make sure formatting is clean
		if(indexOfSep(input) == 0 && indexOfSep(input.substring(indexOfSep(input) + 1)) > 0) { //If string starts with seperator AND this is not the last seperator 
			dataReturn[i] = (input.substring(0, indexOfSep(input.substring(indexOfSep(input) + 1)) + 1)).toFloat(); //Extract float from between next two seperators
			// Serial.println(input.substring(0, indexOfSep(input.substring(indexOfSep(input) + 1)) + 1)); //DEBUG!
			input.remove(0, indexOfSep(input.substring(indexOfSep(input) + 1)) + 1); //Delete leading entry
		}
		// if(indexOfSep(data) == 0) {
		// 	sensorData[i] = (data.substring(0, indexOfSep(data))).toFloat(); //Extract float from between next two seperators
		// 	Serial.println(data.substring(0, indexOfSep(data))); //DEBUG!
		// 	data.remove(0, indexOfSep(data) + 1); //Delete leading entry
		// }
		else {
			input.trim(); //Trim off trailing characters
			dataReturn[i] = input.toFloat();
		}
	}
	return true; //Give pass
}

// void LI710::setTalonPort(uint8_t port)
// {
// 	// if(port_ > numPorts || port_ == 0) throwError(PORT_RANGE_ERROR | portErrorCode); //If commanded value is out of range, throw error 
// 	if(port > 4 || port == 0) throwError(TALON_PORT_RANGE_ERROR | talonPortErrorCode | sensorPortErrorCode); //If commanded value is out of range, throw error //FIX! How to deal with magic number? This is the number of ports on KESTREL, how do we know that??
// 	else { //If in range, update the port values
// 		talonPort = port - 1; //Set global port value in index counting
// 		talonPortErrorCode = (talonPort + 1) << 4; //Set port error code in rational counting 
// 	}
// }

// void LI710::setSensorPort(uint8_t port)
// {
// 	// if(port_ > numPorts || port_ == 0) throwError(PORT_RANGE_ERROR | portErrorCode); //If commanded value is out of range, throw error 
// 	if(port > 4 || port == 0) throwError(SENSOR_PORT_RANGE_ERROR | talonPortErrorCode | sensorPortErrorCode); //If commanded value is out of range, throw error //FIX! How to deal with magic number? This is the number of ports on KESTREL, how do we know that??
// 	else { //If in range, update the port values
// 		sensorPort = port - 1; //Set global port value in index counting
// 		sensorPortErrorCode = (sensorPort + 1); //Set port error code in rational counting 
// 	}
// }

// String LI710::getSensorPortString()
// {
// 	if(sensorPort >= 0 && sensorPort < 255) return String(sensorPort + 1); //If sensor port has been set //FIX max value
// 	else return "null";
// }

// String LI710::getTalonPortString()
// {
// 	if(talonPort >= 0 && talonPort < 255) return String(talonPort + 1); //If sensor port has been set //FIX max value
// 	else return "null";
// }

// int LI710::throwError(uint32_t error)
// {
// 	errors[(numErrors++) % MAX_NUM_ERRORS] = error; //Write error to the specified location in the error array
// 	if(numErrors > MAX_NUM_ERRORS) errorOverwrite = true; //Set flag if looping over previous errors 
// 	return numErrors;
// }

bool LI710::decodeDiag(uint16_t diagCode)
{
	uint16_t warningBits = diagCode & diagWarningMask;
	uint16_t errorBits = diagCode & (~diagWarningMask);

	if(errorBits > 0 || warningBits > 0) { //Only bother to process if one of them is actually set
		for(int i = 0; i < 16; i++) {
			if(errorBits & 0x01) throwError(LI710_ERROR | (i << 8) | talonPortErrorCode | sensorPortErrorCode); //Throw an individual error with unique code for each error present
			if(warningBits & 0x01) throwError(LI710_WARNING | (i << 8) | talonPortErrorCode | sensorPortErrorCode); //Throw an individual warning with unique code for each warning present
			errorBits = errorBits >> 1; //Bit shift vals
			warningBits = warningBits >> 1;
		}
		return true; //Indicate an error 
	}
	return false; //No error present
}

String LI710::getErrors()
{
	// if(numErrors > length && numErrors < MAX_NUM_ERRORS) { //Not overwritten, but array provided still too small
	// 	for(int i = 0; i < length; i++) { //Write as many as we can back
	// 		errorOutput[i] = error[i];
	// 	}
	// 	return -1; //Throw error for insufficnet array length
	// }
	// if(numErrors < length && numErrors < MAX_NUM_ERRORS) { //Not overwritten, provided array of good size (DESIRED)
	// 	for(int i = 0; i < numErrors; i++) { //Write all back into array 
	// 		errorOutput[i] = error[i];
	// 	}
	// 	return 0; //Return success indication
	// }
	String output = "\"LiCor ET\":{"; // OPEN JSON BLOB
	output = output + "\"CODES\":["; //Open codes pair

	for(int i = 0; i < min(MAX_NUM_ERRORS, numErrors); i++) { //Interate over used element of array without exceeding bounds
		output = output + "\"0x" + String(errors[i], HEX) + "\","; //Add each error code
		errors[i] = 0; //Clear errors as they are read
	}
	if(output.substring(output.length() - 1).equals(",")) {
		output = output.substring(0, output.length() - 1); //Trim trailing ','
	}
	output = output + "],"; //close codes pair
	output =  output + "\"OW\":"; //Open state pair
	if(numErrors > MAX_NUM_ERRORS) output = output + "1,"; //If overwritten, indicate the overwrite is true
	else output = output + "0,"; //Otherwise set it as clear
	output = output + "\"NUM\":" + String(numErrors) + ","; //Append number of errors
	output = output + "\"Pos\":[" + getTalonPortString() + "," + getSensorPortString() + "]"; //Concatonate position 
	output = output + "}"; //CLOSE JSON BLOB
	numErrors = 0; //Clear error count
	return output;

	// return -1; //Return fault if unknown cause 
}