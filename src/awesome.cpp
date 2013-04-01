/* awesome.cpp
*  Removes new lines and line comments from files, leaving them in only when absolutely necessary
*  Author: Jake Tucker (Do I really want to be associated with this?)
*/

#include <iostream>
#include <fstream>
#include <string>
#include <cstring>

void notDangerZone(char *inputFilename, char *outputFilename);
void dangerZone(int argc, char **argv);
void removeNewLines(std::ifstream &inputFile, std::ofstream &outputFile);
void copyFile(std::ifstream &inputFile, std::ofstream &outputFile);

int main(int argc, char **argv)
{
	// Check if we are entering a zone of danger (but only if there are enough command line arguments for that to be a possibility)
	bool dangerzone = 0;
	if(argc > 1)
	{
		if(std::strcmp(argv[1], "--dangerzone") == 0)
			dangerzone = 1;
	}

	// If incorrect number of arguments, exit the program
	if(argc < 3)
	{
		std::cout << "Usage: " << argv[0] << " [options] input output" << std::endl
			<< "Options:" << std::endl
			<< "  --dangerzone <files>    Permanently changes all arguments" << std::endl;
		return -1; // Because who needs standards?
	}

	if(!dangerzone)
		notDangerZone(argv[1], argv[2]); // Open file 1, remove newline characters, send to file 2
	else
		dangerZone(argc - 2, &(argv[2])); // Send arguments to the danger zone

	return 0;
}

void notDangerZone(char *inputFilename, char *outputFilename)
{
	// Open input file
	std::ifstream inputFile;
	inputFile.open(inputFilename);

	// Open output file
	std::ofstream outputFile;
	outputFile.open(outputFilename);

	// Removes new lines from file
	removeNewLines(inputFile, outputFile);
	outputFile << std::endl;

	// Close files
	inputFile.close();
	outputFile.close();
}

void dangerZone(int argc, char **argv)
{
	int i;
	std::ifstream inputFile;
	std::ofstream outputFile;
	std::string tempFileName;

	for(i=0; i<argc; i++)
	{
		// We write to a temp file while removing new lines
		tempFileName = argv[i];
		tempFileName = tempFileName + "~";
		
		// Open input file and temp file
		inputFile.open(argv[i]);
		outputFile.open(tempFileName.c_str());

		//Remove new lines
		removeNewLines(inputFile, outputFile);
		outputFile << std::endl;

		//Close files
		inputFile.close();
		outputFile.close();

		// Open temp file and original file
		inputFile.open(tempFileName.c_str());
		outputFile.open(argv[i]);

		// Copy temp to new file
		copyFile(inputFile, outputFile);

		// Close files
		inputFile.close();
		outputFile.close();

		// Delete temp file
		std::remove(tempFileName.c_str());

		// Lets the user know which files they made awesome today
		std::cout << argv[i] << " made awesome" << std::endl;
	}
}

void removeNewLines(std::ifstream &inputFile, std::ofstream &outputFile)
{
	char tempChar = 0;
	std::string tempString = "";
	std::streampos pos = 0;

	/* 
	* Keeps track of whether there was a new line previously so that the the preprocessor
	* statement detection knows whether or not it needs to add a new line before outputting
	* the statement to the new, more awesome file. This variable is static because it can be.
	*/
	static bool newLinePrevious = 1;

	while(inputFile.good())
	{
		tempChar = inputFile.get();

		/* 
		* If line is a preprocessor directive, a new line is required for the program to still
		* compile (unfortunately).
		*/
		if(tempChar == '#')
		{
			pos = inputFile.tellg();
			std::getline(inputFile, tempString);
			if(tempString.find("include ") != std::string::npos
				|| tempString.find("define ") != std::string::npos
				|| tempString.find("if ") != std::string::npos
				|| tempString.find("elif ") != std::string::npos
				|| tempString.find("endif ") != std::string::npos
				|| tempString.find("ifdef ") != std::string::npos
				|| tempString.find("ifndef ") != std::string::npos
				|| tempString.find("undef ") != std::string::npos)
			{
				// If this is the first space in the program and it's a #, it's a preprocessor directive (in all likelihood)
				if(pos == std::streampos(1))
				{
					if(!newLinePrevious)
						outputFile << std::endl;
					outputFile << '#' << tempString << std::endl;
					newLinePrevious = 1;
				}
				/* 
				* This is a convoluted way of checking what came before the # we just read
				* If this is a new line, then this is a PPD, and so we act accordingly
				* Otherwise, continue writing normally through recursive black magic
				* I'll be honest, I can't remember why I thought this would work.
				*/
				else
				{
					inputFile.seekg(pos); // Move to just after '#'
					inputFile.seekg(-2, inputFile.cur); // Move to just before '#'
					tempChar = inputFile.get(); // Read character before '#', move to '#'
					inputFile.seekg(+1, inputFile.cur); // Move back to just after '#'
					std::getline(inputFile, tempString); // Read rest of line, move to original position
					if(tempChar == '\n') // If the steps above retrieved a newline, this is likely a PPD
					{
						if(!newLinePrevious)
							outputFile << std::endl;
						outputFile << '#' << tempString << std::endl;
						newLinePrevious = 1;
					}
					else
					{
						inputFile.seekg(pos);
						outputFile << '#';
						newLinePrevious = 0;
						removeNewLines(inputFile, outputFile);
					}
				}
			}
			else
			{
				inputFile.seekg(pos);
				outputFile << '#';
				newLinePrevious = 0;
				removeNewLines(inputFile, outputFile);
			}
		}
		// Line comments comment out the rest of the line (and so the entire program!), this detects that and removes those comments
		else if(tempChar == '/')
		{
			tempChar = inputFile.get();
			if(tempChar == '/')
				std::getline(inputFile, tempString);
			else
			{
				outputFile << '/';
				if('\n' != tempChar && inputFile.good() )
					outputFile << tempChar;
			}
			newLinePrevious = 0;
		}
		// If there is nothing special about this character, check for newline, write if it isn't
		else if('\n' != tempChar && inputFile.good() )
		{
			outputFile << tempChar;
			newLinePrevious = 0;
		}
	}
}

// Reads character from input, if this was not EOF garbage, the character is written to the output
void copyFile(std::ifstream &inputFile, std::ofstream &outputFile)
{
	char tempChar = 0;
	while(inputFile.good())
	{
		tempChar = inputFile.get();
		if(inputFile.good())
			outputFile << tempChar;
	}
}
