#include <iostream>
#include <fstream>
#include <cmath>
#include <climits>
using namespace std;

enum Inst{
	comment, codeDir, dataDir, label, LD, LDi, SD, SDi, ADD, SUB, MUL, DIV, ADDi, SUBi, MULi, DIVi, JMP, JZ, JNZ, JGZ, JLZ, JGEZ, JLEZ, undecided
};
enum nextNeed{
	nReg, nMemLoc, nNum, nMemReg, nPC
};
void updateStatistic(Inst* statisticType, unsigned int* instrucCounter){
	switch(*statisticType){
			case LD:
			case SD:
			case LDi:
			case SDi:
				instrucCounter[0]++;
				break;
			case MUL:
			case DIV:
			case ADD:
			case SUB:
			case MULi:
			case DIVi:
			case ADDi:
			case SUBi:
				instrucCounter[1]++;
				break;
			case JMP:
			case JZ:
			case JNZ:
			case JGZ:
			case JGEZ:
			case JLZ:
			case JLEZ:
				instrucCounter[2]++;
				break;
			default:;
	}
}

bool strComp(char* str1, char* str2){
	unsigned int len1 = 0;
	unsigned int len2 = 0;
	while (str1[len1] != 0)
		len1++;
	while (str2[len2] != 0)
		len2++;
	if (len1 != len2)
		return false;
	else{
		for (int letter = 0; letter < len1; letter++){
			if (str1[letter] != str2[letter])
				return false;
		}
	}
	return true;
}

bool numCheck(char* num, nextNeed instrucNeed){
	unsigned int digit = 0;
	if (((instrucNeed == nNum) && (num[digit] == '+' || num[digit] == '-' || ((int)num[digit] >= (int)'0' && (int)num[digit] <= (int)'9'))) || ((instrucNeed != nNum) && (num[digit] == '+' || ((int)num[digit] >= (int)'0' && (int)num[digit] <= (int)'9')))){
		digit++;
		while (num[digit] != '\0'){
			if (!((int)num[digit] >= (int)'0' && (int)num[digit] <= (int)'9'))
				return true;	// if not a number, exit
			digit++;
		}
		return false;
	}
	return true; // if not a number, exit
}

void strCpy(char* str1, char* str2){
	unsigned int len2 = 0;
	while (str2[len2] != '\0')
		len2++;
	for (int letter = 0; letter < len2; letter++){
		str1[letter] = str2[letter];
	}
	str1[len2] = '\0';
}

int convToInt (char* wordToConvert, unsigned int strLen){
	int asciiDiff = (int)'0';
	int returnNum = 0;
	int tempNum;
	for (int digit = 0; digit < strLen; digit++){
		tempNum = (int)wordToConvert[digit];
		if (tempNum >= (int)'0' && tempNum <= (int)'9'){
			tempNum -= asciiDiff;	// yields the actual numerical value
			returnNum += tempNum * pow(10, strLen - digit - 1);
		}
		else{
			cerr << "Error: invalid input. Not an integer." << endl;
			return -1;
		} 
	}
	return returnNum;
}

bool checkLabel (char* label){
	unsigned int letter = 0;
	if (((int)label[letter] >= (int)'a' && (int)label[letter] <= (int)'z') || ((int)label[letter] >= (int)'A' && (int)label[letter] <= (int)'Z')){
		letter++;
		while (label[letter] != 0){
			if (((int)label[letter] >= (int)'a' && (int)label[letter] <= (int)'z') || ((int)label[letter] >= (int)'A' && (int)label[letter] <= (int)'Z') || ((int)label[letter] >= (int)'a' && (int)label[letter] <= (int)'z')){
				letter++;}
			else
				return false;
		}
	}
	else
		return false;
	return true;
}

int main (int argc, char* argv[]){
	ifstream infile;

	unsigned int instrucTypeCount[3] = {0, 0, 0};	// load, math, jumping
	unsigned int lineNum = 0;
	unsigned int labelTotal = 0;
	unsigned int codeDirLinePosition = INT_MAX;
	int codeValue = -1;
	int dataValue = -1;
	const unsigned int maxFirstStLength = 28;	// holds 9 elements
	const unsigned int maxOperandLength = 247; // holds 245 elements
	const unsigned int maxLineLength = 256;
	const unsigned int maxNumLabels = 30;
	char** labels = new char*[maxNumLabels];
	unsigned int labelLineNum[maxNumLabels];
	unsigned int curAssemblyLine;
	char fileLine[maxLineLength];
	bool shouldExit = false;
	Inst curInstruc;
	// assume the file name's input is valid
	if (argc != 2){
		cerr << "Error: only one command-line argument is required." << endl;
		return 1;
	}
	infile.open(argv[1]);
	if (infile.is_open()){	// Read file lines once and store labels into array
		const unsigned int labelMaxLength = 28;
		while (!infile.eof()){
			 // ****** good
		bool alreadyExist = false;
		infile.getline(fileLine, maxLineLength);
		lineNum++;
		char firstSt[maxFirstStLength] = {'\0'};
		unsigned int letter = 0;
		unsigned int firstStLength = 0;
			while(fileLine[letter] == ' ' || fileLine[letter] == '\t'){ // read spaces until we get a letter
				letter++;
			} 
			if (fileLine[letter] == '\0')								// if blank line is detected, move on
				continue;
			do{																// load and store the first word
				if (firstStLength >= maxFirstStLength - 1){
					shouldExit = true;
					break;
				}
				firstSt [firstStLength] = fileLine[letter];
				firstStLength++; // length of the OPCODE
				letter++;		// letter of the line we are at
			}while (fileLine[letter] != ' ' && fileLine[letter] != '\0' && fileLine[letter] != '\t' && fileLine[letter] != ':');
			if (shouldExit)			// length exceeds boundaries
				break;
		if (fileLine[letter] == ':'){
			firstSt[firstStLength] = '\0'; // make a null terminator
			if (strComp(firstSt, "Code")){
				curInstruc = codeDir;
				codeDirLinePosition = lineNum;	// memorize the position of the code directive
			}
			else if (strComp(firstSt, "Data")){
				if (codeDirLinePosition > lineNum)
					cerr << "Error on line " << lineNum << ": misplaced directive " << endl; // ******** this may cause complications
				curInstruc = dataDir;
			}
			else{
				curInstruc = label;
			} 
			if (curInstruc != label){ // if directive detected
				letter++;
				unsigned int secondStrCounter = 0;
				char tempStr[10] = {'\0'};
				while(fileLine[letter] == ' ' || fileLine[letter] == '\t'){ // read spaces until we get a letter
					letter++;
				} 
				do{																// load and store the first word
					if (secondStrCounter >= maxFirstStLength - 1){
						shouldExit = true;
						break;
					}
					tempStr [secondStrCounter] = fileLine[letter];
					secondStrCounter++; // length of the OPCODE
					letter++;		// letter of the line we are at
				}while (fileLine[letter] != ' ' && fileLine[letter] != '\0' && fileLine[letter] != '\t');
				tempStr[secondStrCounter] = '\0';
				int tempNum = convToInt(tempStr, secondStrCounter);
				if (tempNum != -1){
					if (curInstruc == codeDir){
						if (codeValue != -1) // in this case, code value was already assigned
							cerr << "Error on line " << lineNum << ": duplicate code directive detected " << endl;
						else{ 				// if -1 (never reassigned), assign the new value
							codeValue = tempNum;
							curAssemblyLine = codeValue;
						}
					}
					else if (curInstruc == dataDir){
						if (dataValue != -1) // in this case, data value was already assigned
							cerr << "Error on line " << lineNum << ": duplicate data directive detected " << endl;
						else
							dataValue = tempNum;
				}
				else{
					cerr << "Error on line " << lineNum << ": invalid input. Expected integer as input " << endl;
				}
				}
			}
			else{
				bool labelValid = checkLabel(firstSt);
				if (!labelValid){
					cerr << "Error on line " << lineNum << ": label is invalid " << endl;
				}
				for (int label = 0; label < labelTotal; label++) { 		// this for loop compares all pre-loaded labels to the current label in question
					alreadyExist = strComp(labels[label], firstSt);
					if (alreadyExist){
						cerr << "Error on line " << lineNum << ": duplicate label detected " << endl;
						break;
					}
				}
				if (alreadyExist == false){
					labels[labelTotal] = new char[labelMaxLength];
					strCpy(labels[labelTotal], firstSt);	
					labelTotal++;
				}
			}// if label was detected
		} // if statement: if : detected
		shouldExit = false;
		}// outer while loop
		if (codeValue == -1)
			cerr << "Error on line " << lineNum << ": missing code directive " << endl;
	}
	else{
		cerr << "Error: unable to open file " << endl;
		return 1;
	}
	infile.close();
	infile.open(argv[1]);
	if (infile.is_open()){
		lineNum = 0;
		shouldExit = false;
		unsigned int operandChecks;
		unsigned int numLabelsPassed = 0;
		while(!infile.eof()){
			infile.getline(fileLine, maxLineLength);
			// cerr << fileLine << endl; // ******* remove this after
			operandChecks = 0;
			char firstSt[maxFirstStLength] = {'\0'};
			char operands[maxOperandLength] = {'\0'};
			bool hasColon = false;
			unsigned int letter = 0;
			unsigned int firstStLength = 0;
			lineNum++; // assume blank lines are a line
			while(fileLine[letter] == ' ' || fileLine[letter] == '\t'){ // read spaces until we get a letter
				letter++;
			} 
			if (fileLine[letter] == '\0')								// if blank line is detected, move on
				continue;
			do{																// load and store the first word
				firstSt [firstStLength] = fileLine[letter];
				firstStLength++; // length of the OPCODE
				letter++;		// letter of the line we are at
			}while (fileLine[letter] != ' ' && fileLine[letter] != '\0' && fileLine[letter] != '\t' && fileLine[letter] != ':');
			if (fileLine[letter] == 0){
				shouldExit = true;
				break;
			}			// length exceeds boundaries
			if (fileLine[letter] == ':')
				hasColon = true;
			firstSt [firstStLength] = '\0'; // make a null terminator
			if (firstSt[0] == '#'){			// if comment, ignore all from the get go
				curInstruc = comment;
				continue;
			}
			curInstruc = undecided; 	// continuous initializations
			if (hasColon){
				if (!strComp(firstSt, "Code") && !strComp(firstSt, "Data")){ // PC value for instruction. ******* For now, skip over these lines
					labelLineNum[numLabelsPassed] = curAssemblyLine;
					numLabelsPassed++;
				}
				continue;
			}
			else{								// Actual instruction is contained on the line
				if (codeDirLinePosition > lineNum)
					cerr << "Error on line " << lineNum << ": missing code directive " << endl;
				if (firstSt[0] == 'J'){
					switch(firstStLength){
						case 4:
							operandChecks = 3;
							if (strComp(firstSt, "JGEZ")){
								curInstruc = JGEZ;
							}
							else if (strComp(firstSt, "JLEZ")){
								curInstruc = JLEZ;
							}
							break;
						case 3:
							if (strComp (firstSt, "JNZ")){
								operandChecks = 3;
								curInstruc = JNZ;
							}
							else if (strComp (firstSt, "JGZ")){
								operandChecks = 3;	
								curInstruc = JGZ;								
							}
							else if (strComp(firstSt, "JLZ")){
								operandChecks = 3;
								curInstruc = JLZ;
							}
							else if (strComp(firstSt, "JMP")){
								operandChecks = 2;
								curInstruc = JMP;
							}
							break;
						default: 
							if (strComp (firstSt, "JZ")){
								operandChecks = 3;
								curInstruc = JZ;
							}
					}
				}
				else if (firstSt[firstStLength - 1] == 'i'){
					switch(firstStLength){
						case 4: 
							operandChecks = 4;
							if (strComp(firstSt, "ADDi")){
								curInstruc = ADDi;
							}
							else if (strComp(firstSt, "SUBi")){
								curInstruc = SUBi;
							}
							else if (strComp(firstSt, "MULi")){
								curInstruc = MULi;
							}
							else if (strComp(firstSt, "DIVi")){
								curInstruc = DIVi;
							}
							break;
						case 3:
							operandChecks = 3;
							if (strComp(firstSt, "LDi")){
								curInstruc = LDi;
							}
							else if (strComp(firstSt, "SDi")){
								curInstruc = SDi;
							}
							break;
					}
				}
				else{
					switch(firstStLength){
						case 3:
							operandChecks = 4;
							if (strComp(firstSt, "MUL")){
								curInstruc = MUL;
							}
							else if (strComp(firstSt, "DIV")){
								curInstruc = DIV;
							}
							else if (strComp(firstSt, "ADD")){
								curInstruc = ADD;
							}
							else if (strComp(firstSt, "SUB")){
								curInstruc = SUB;
							}
						break;
						case 2:
							operandChecks = 3;
							if (strComp(firstSt, "LD")){
								curInstruc = LD;
							}
							else if (strComp(firstSt, "SD")){
								curInstruc = SD;
							}				
						}
					}
			} // end of part 1
		if (curInstruc == undecided){	// Error message for wrong opcode
			cerr << "Error on line " << lineNum << ": OPCODE, label, or directive may be invalid " << endl;
			continue;
		}
		nextNeed* instrucNeed = new nextNeed[operandChecks - 1];

		// copy the other portions 
		switch(curInstruc){ // now, I'm only accomodating for actual assembly instructions
			case MUL:
			case DIV:
			case ADD:
			case SUB:
				instrucNeed[0] = nReg;
				instrucNeed[1] = nReg;
				instrucNeed[2] = nReg;
				break;
			case MULi:
			case DIVi:
			case ADDi:
			case SUBi:
				instrucNeed[0] = nReg;
				instrucNeed[1] = nNum;
				instrucNeed[2] = nReg;
				break;
			case JMP:
				instrucNeed[0] = nPC;
				break;
			case JZ:
			case JNZ:
			case JGZ:
			case JGEZ:
			case JLZ:
			case JLEZ:
				instrucNeed[0] = nReg;
				instrucNeed[1] = nPC;
				break;
			case LD:
				instrucNeed[0] = nMemReg;
				instrucNeed[1] = nReg;
				break;
			case SD:
				instrucNeed[0] = nReg;
				instrucNeed[1] = nMemReg;
				break;
			case LDi:
				instrucNeed[0] = nNum;
				instrucNeed[1] = nReg;
				break;
			case SDi:
				instrucNeed[0] = nNum;
				instrucNeed[1] = nMemReg;
				break;
			default:; // ****** change this later
		}
		if (operandChecks != 0){	
			curAssemblyLine++;
		}		// Checking all operands for validity
		for (int check = 1; check < operandChecks; check++){ 
			bool needReg = false;
			bool needLabel = false;
			const unsigned int numDigitMax = 30;
			unsigned int checkedNumLength = 0;
			char checkedNum[numDigitMax] = {'\0'};				// can hold 5 digits rn
			if (check > 1){
				while ((fileLine[letter] == ' ' || fileLine[letter] == '\t') && fileLine[letter] != '\0') {	// skip white spaces
					letter++;
				}
				switch (fileLine[letter]){																	// check for presence of comma
					case ',':
						break;
					case '\0':
						cerr << "Error on line " << lineNum << ": Missing OPERAND " << check << endl;
					default:
						shouldExit = true;
						break;
				}
				letter++;
				if (shouldExit){ // if no comma is detected, ignore the rest of the line
					cerr << "Error on line " << lineNum << ": invalid due to missing comma " << endl;
					break;
				}
			}
			while ((fileLine[letter] == ' ' || fileLine[letter] == '\t') && fileLine[letter] != '\0') {	// skip white spaces
				letter++;
			}
			if (fileLine[letter] == '\0'){						// if end of line, ignore rest of line
				shouldExit = true;
				cerr << "Error on line " << lineNum << ": Missing OPERAND " << check << endl;
				break;
			}
			char errorType[30] = {'\0'};
			bool isWrongRegister = false;

			if (instrucNeed[check - 1] == nPC){ // the special case where PC is a label in square brackets
				if (fileLine[letter] == '['){
					needLabel = true;
					letter++;
				}
			}
				if (instrucNeed[check - 1] == nReg && fileLine[letter] != 'R'){ // if we need a register but don't encounter one, invalid
					isWrongRegister = true;
				}
				if ((instrucNeed[check - 1] == nReg || instrucNeed[check - 1] == nMemReg) && !isWrongRegister){	// in these states/ conditions, we may need a register
					if (fileLine[letter] == 'R'){
						needReg = true;
						letter++;
					}
				}
				switch(instrucNeed[check - 1]){		// load up an error messages
					// number checking should come last. If the nMemReg == true, iterate if we are dealing with a "R". Otherwise, proceed directly to check for numbers
					case nReg:
						strCpy(errorType, "Register ");
						break;
					case nMemReg:
						strCpy(errorType, "Register or Memory Location ");
						break;
					case nMemLoc: // later, check whether the number is valid. This may require another function
						strCpy(errorType, "Memory Location ");
						break;
					case nNum:
						strCpy(errorType, "Integer ");
						break;
					case nPC:
						strCpy(errorType, "Program Counter Value ");
				}
						do{				// check for assembly instruction validity. Here, load up the number one-by-one.
							checkedNum[checkedNumLength] = fileLine[letter];
							letter++;
							checkedNumLength++;
						}while (fileLine[letter] != ' ' && fileLine[letter] != '\t' && fileLine[letter] != ',' && fileLine[letter] != '\0' && checkedNumLength < numDigitMax - 1); // instruction can be borderedlined by any of the above
						if (fileLine[letter] == '\0' && check != operandChecks - 1){	// if this is the second last iteration, having \0 is valid. Proceed to checking operand validity
							cerr << "Error on line " << lineNum << ": missing several OPERANDS." << endl; 	// **** Currently no way of differentiating between 3 errors
							shouldExit = true; 				
							break;
						}
						checkedNum[checkedNumLength] = '\0';
						if (needLabel){					// check for label validity ******* This is the part in question
							if (checkedNum[checkedNumLength - 1] == ']'){
								checkedNum[checkedNumLength - 1] = '\0';
								bool labelValid = checkLabel(checkedNum);
								if (!labelValid){
									cerr << "Error on line " << lineNum << ": label is invalid " << endl;
								}
								shouldExit = true;
								for (int label = 0; label < labelTotal; label++) { 		// this for loop compares all pre-loaded labels to the current label in question
									if (strComp(labels[label], checkedNum)){
										shouldExit = false;
										break;
									}
								}
							}
							else
								shouldExit = true;
						}
						else
							shouldExit = numCheck(checkedNum, instrucNeed[check - 1]);
						if (shouldExit || isWrongRegister){
							shouldExit = true;
							if (needReg)
								cerr << "Error on line " << lineNum << ": invalid input. Expected " << errorType << "for OPERAND " << check << "; saw \"R" << checkedNum << "\"" << endl;
							else
								cerr << "Error on line " << lineNum << ": invalid input. Expected " << errorType << "for OPERAND " << check << "; saw \"" << checkedNum << "\"" << endl;
							break;// **** pay attention to end-bound checking.
								  // Must implement limit checking later
						}
				if (shouldExit)
					break;
			} 	// inner for loop
			delete[] instrucNeed;
			if (!shouldExit){ 	// the syntax has been valid so far. Check the part after the last operand
				while (fileLine[letter] != '\0' && shouldExit == false){
					switch (fileLine[letter]){
						case '\t':
						case ' ':
							letter++;
							break;
						case '#':	// update statistic here
							updateStatistic(&curInstruc, instrucTypeCount);
							shouldExit = true;
							break;
						default:
							shouldExit = true;
							cerr << "Error on line " << lineNum << ": Extra data after instruction." << endl;
					}
				}
			}
			if (!shouldExit) // update statistic here
				updateStatistic(&curInstruc, instrucTypeCount);
		shouldExit = false;
		} 	// while loop
		if (shouldExit)
			cerr << "Error on line " << lineNum << ": missing OPERAND or file line is too long to parse " << endl;
		if (curInstruc == label)
			labelLineNum[numLabelsPassed - 1] = curAssemblyLine + 1; // ******* this may cause complications
		else{
			cout << "Code: " << codeValue << endl;
			if (dataValue != -1)
				cout << "Data: " << dataValue << endl;
			for (int label = 0; label < labelTotal; label++)
				cout << labels[label] << ": " << labelLineNum[label] << endl;
			cout << "Total number of assembly instructions: " << instrucTypeCount[0] + instrucTypeCount[1] + instrucTypeCount[2] << endl;
			cout << "Number of Load/Store: " << instrucTypeCount[0] << endl;
			cout << "Number of ALU: " << instrucTypeCount[1] << endl;
			cout << "Number of Compare/Jump: " << instrucTypeCount[2] << endl;
		}
	}	// if statement
	else{
		cerr << "Error: Unable to open file." << endl;
	}
}
