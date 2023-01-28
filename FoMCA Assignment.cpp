// The text encryption program in C++ and ASM with a very simple example encryption method - it simply adds 1 to the character.
// The encryption method is written in ASM. You will replace this with your allocated version for the assignment.
// In this version parameters are passed via registers (see 'encrypt' for details).
//
// Original Author: Mrat Frezghi
// Last revised: Jan 2022

#include <string>     // for std::string
#include <chrono>     // for date & time functions
#include <ctime>      // for date & time string functions
#include <iostream>   // for std::cout <<
#include <fstream>    // for file I/O
#include <iomanip>    // for fancy output
#include <functional> // for std::reference_wrapper
#include <vector>     // for std::vector container

constexpr char const * STUDENT_NAME = "Mrat Frezghi";		 // my full name
constexpr int ENCRYPTION_ROUTINE_ID = 22;                    // my encryption id
constexpr char ENCRYPTION_KEY = 'D';                         // my encryption key to be used to encrypt characters
constexpr int MAX_CHARS = 6;                                 // max characters that user can input

constexpr char STRING_TERMINATOR = '$';                      // custom string terminator
constexpr char LINE_FEED_CHARACTER = '\n';                   // line feed character 
constexpr char CARRIAGE_RETURN_CHARACTER = '\r';             // carriage return character

char original_chars[MAX_CHARS];								 // Original character string
char encrypted_chars[MAX_CHARS];                             // Encrypted character string
char decrypted_chars[MAX_CHARS];							 // Decrypted character string


//---------------------------------------------------------------------------------------------------------------
//----------------- C++ FUNCTIONS -------------------------------------------------------------------------------

/// <summary>
/// get a single character from the user via Windows function _getwche
/// </summary>
/// <param name="a_character">the resultant character, pass by reference</param>
void get_char(char& a_character)
{
	a_character = (char)_getwche(); // https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/getche-getwche

	if (a_character == STRING_TERMINATOR) // skip further checks if user entered string terminating character
	{
		return;
	}
	if (a_character == LINE_FEED_CHARACTER || a_character == CARRIAGE_RETURN_CHARACTER)  // treat all 'new line' characters as terminating character
	{
		a_character = STRING_TERMINATOR;
		return;
	}
	if (a_character <= 91) {			//looks at the character to see if any of the values goes below 91 since that is thats all the capital letters value are 65-91
		a_character = a_character + 32; //if it is a capital letters then it add 32 since a = 65, A = 97 having a 32 diffrence so it make it lower 
		return;
	}
}

//---------------------------------------------------------------------------------------------------------------
//----------------- ENCRYPTION ROUTINE --------------------------------------------------------------------------

/// <summary>
/// 'encrypt' the characters in original_chars, up to length
/// output 'encrypted' characters in to encrypted_chars
/// </summary>
/// <param name="length">length of string to encrypt, pass by value</param>
/// <param name="EKey">encryption key to use during encryption, pass by value</param>
void encrypt_chars(int length, char EKey)
{
	char temp_char;						   // Temporary character store

	for (int i = 0; i < length; ++i)	   // Encrypt characters one at a time
	{
		temp_char = original_chars[i];	   // Get the next char from original_chars array

		__asm
		{

			push   eax                     // save orginal value of these registers since they are going to be used
			push   ecx                     // moves ecx value to the stack
			push   edx                     // moves edx value to the stack

			lea    eax, EKey               // transfers Ekey address to eax which will be later alterd using al
			movzx  ecx, temp_char          // moves the chracter that needs to be encrypted to the whole ecx and changes it into a dword
			push   ecx
			call   encrypt_22              // calls the encryption method
			add	   esp, 4                  // scrubs the stack 
			mov    temp_char, dl           // adds the encrypted character to temp_char

			pop    edx                     // takes the orginal value of edx out of the stack and restores to orginal value to not cause issues
			pop    ecx                     // takes the orginal value of ecx out of the stack
			pop    eax                     // takes the orginal value of eax out of the stack

		}

		encrypted_chars[i] = temp_char;	   // Store encrypted char in the encrypted_chars array
	}

	return;

	// Inputs: register EAX = 32-bit address of Ekey
	//                  ECX = the character to be encrypted (in the low 8-bit field, CL)
	// Output: register EDX = the encrypted value of the source character (in the low 8-bit field, DL)

	__asm
	{
	encrypt_22:
			push  ebp							// save the value of the orginal base pointer to the stack
			mov   ebp, esp						// Set up the new position of the base pointer

			mov ecx, [ebp + 8]					// gets the value of temp_char out the stack and on to ecx
			//---------------------
			push  edi							// pushes edi to the stack to revert it at the end
			push  ecx							// pushes ecx to the stack to revert it at the end
			and   byte ptr[eax], 0x0F			// changes the value of Ekey anding it too 15 
			cmp   byte ptr[eax], 0x00			// looks at the Ekey and comparing it to 0 if its not it the code jumps, if it does it skips the jump
			jnz   x22							// goes to the subroutine z22 
			mov   edi, 0x02						// transfers 2 into edi only if byte Ekey didnt = to 0
			jmp   z22							// goes to the subroutine z22 after the operation goes off
			x22 : movzx edi, byte ptr[eax]		// moves Ekey into the whole of edi making it 32bits also used for how many times its going to loop y22
			z22 : add   byte ptr[eax], 0x0A		// it adds to the Ekey 10 
			pop   eax							// restore the orginal value of eax since its not being used anymore
			y22 : dec   eax						// takes away one from eax to push the letter back by one
			dec   edi							// takes one away from edi creating a loop
			jnz   y22							// jumps to y22 until edi = 0
			not al								// flips the individual bits of al
			dec   eax							// takes away 1 from eax changing the value of al
			pop   edi							// restore the orginal value of edi since its not being used anymore
			xor   al, 0x0A						// takes away 10 from al value
			mov   edx, eax						// copies eax onto edx which will used to get the encrypted ascii value
			//----------------
			mov   esp, ebp						// make sure the stack pointer is at the same point as the base pointer
			pop   ebp							// gets the orginal value of ebp to not cause issues with other code that uses it
			ret									// returns to the for loop

	}
}
//*** end of encrypt_chars function
//---------------------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------------------
//----------------- DECRYPTION ROUTINE --------------------------------------------------------------------------

/// <summary>
/// 'decrypt' the characters in encrypted_chars, up to length
/// output 'decrypted' characters in to decrypted_chars
/// </summary>
/// <param name="length">length of string to decrypt, pass by value</param>
/// <param name="EKey">encryption key to used during the encryption process, pass by value</param>
void decrypt_chars(int length, char EKey)
{
	char temp_char;						   // create a vrablie to hold the chracters its going to decrypt

	for (int i = 0; i < length; ++i)	   // use the value of I to get each characters of the encrypted sting
	{
		temp_char = encrypted_chars[i];	   // get the character to decrypt and uses I to get the specific value

		__asm
		{

			push	eax                     // before doing anything pushing is going to put each register value 
			push	ecx                     // in the stack to be be stored so that when im done with them I can  
			push	edx                     // return them to there orginal value incase it contains important data

			lea		ecx, EKey               // move the address of the ENCRYPTION_KEY to ecx to be used within the decryption since it was used to encrypt
			movzx	eax, temp_char          // also moving the character that is getting decrypted into eax
			push	ecx						// be used later on to get the Ekey value from the stack
			call	decrypt_22              // go the the decrypt function
			mov		temp_char, dl           // retrive the now decrypted value from edx (dl is the bottom 8bit value of edx)

			pop		edx                     // we use pop to get get back the orinal value
			pop		ecx                     // of edx, ecx, eax, and doing it reverse order 
			pop		eax                     // then the push since that how you retreve it of the stack

		}

		decrypted_chars[i] = temp_char;	   // adds on the decrypted chracter until it reaches the end of the length
	}

	return;

	__asm
	{
	decrypt_22:
		push		ebp								//  move the base pointer to the stack to be later restored after use
			mov		ebp, esp						// line up the base pointer with the stack pointer to set up a new fixed value
			mov		ecx, [ebp + 8]					// retrieve the value  
			push	edi								// saving the value of both edi and ecx
			push	ecx								// to be later restored once im done using them
			//---------------------
			xor		al, 0x0A						// this compares each bit of al to each bit of 10 so if the first bit has a number it changes it and flips its value and if the second bit of al is not 0 it leaves it  
			add		eax, 0x01						// in the encryption it used subtraction so here we do addtion of the same value, 1
			not		al								// takes each number in al and flips them so it its 1 it changes to 0 and since its hex it would flip B to 4
			and		byte ptr[ecx], 0x0F				// looks at the byte of ecx and compares it to 15 if its also 15 it changes it value to 0 since in hex the max value is 15 so this is to not cause issues
			cmp		byte ptr[ecx], 0x00				// if the and chnages it to 0 then it will skip the jump funation since now we need ecx to have value to continue
			jnz		x22								//  goes to x22 if ecx contains a value 
			mov		edi, 0x02						// if ecx = 0 then we cant use it to set up edi so instead we just add 2 onto edi
			jmp		z22								// once its been added on we wanna skip passed x22 since we dont need to do that and it could cause issues if we did
			x22	:	movzx edi, byte ptr[ecx]		// since ecx conatines a value below 15 we can use on edi
			z22 :	add   byte ptr[ecx], 0x0A		// after we are done setting up edi we give the byte ptr[ecx] a vlue so its not 0 
			y22 :	add   eax, 1					// the y22 is the start of a loop which will take one away from the eax(encrypted character) until edi = 0
			dec		edi								// takes 1 away from edi until it reaches 0
			jnz		y22								// gose back up to y22 until edi = 0
			mov		edx, eax						// takes the now decrypted value frin eax on to edx
			//----------------
			mov		esp, ebp						// make sure the stack pointer is at the same point as the base pointer
			pop		ebp								// restore the original position of the base pointer in case it is being used somewhere else
			ret		4								// exit the functiom and the 4 is used to srub the stack and since we only used one value that is 4 bit we srub 4 bits of the stack
	}
}
//*** end of decrypt_chars function
//---------------------------------------------------------------------------------------------------------------





//************ MAIN *********************************************************************************************

void get_original_chars(int& length)
{
	length = 0;

	char next_char;
	do
	{
		next_char = 0;
		get_char(next_char);
		if (next_char != STRING_TERMINATOR)
		{
			original_chars[length++] = next_char;
		}
	} while ((length < MAX_CHARS) && (next_char != STRING_TERMINATOR));
}

std::string get_date()
{
	std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	char buf[16] = { 0 };
	tm time_data;
	localtime_s(&time_data, &now);
	std::strftime(buf, sizeof(buf), "%d/%m/%Y", &time_data);
	return std::string{ buf };
}

std::string get_time()
{
	std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	char buf[16] = { 0 };
	tm time_data;
	localtime_s(&time_data, &now);
	std::strftime(buf, sizeof(buf), "%H:%M:%S", &time_data);
	return std::string{ buf };
}

// support class to help output to multiple streams at the same time
struct multi_outstream
{
	void add_stream(std::ostream& stream)
	{
		streams.push_back(stream);
	}

	template <class T>
	multi_outstream& operator<<(const T& data)
	{
		for (auto& stream : streams)
		{
			stream.get() << data;
		}
		return *this;
	}

private:
	std::vector <std::reference_wrapper <std::ostream>> streams;
};

int main()
{
	int char_count;  // The number of actual characters entered (upto MAX_CHARS limit)

	std::cout << "Please enter upto " << MAX_CHARS << " alphabetic characters: ";
	get_original_chars(char_count);	// Input the character string to be encrypted


	//*****************************************************
	// Open a file to store results 

	std::ofstream file_stream;
	file_stream.open("log.txt", std::ios::app);
	file_stream << "Date: " << get_date() << " Time: " << get_time() << "\n";
	file_stream << "Name:                  " << STUDENT_NAME << "\n";
	file_stream << "Encryption Routine ID: '" << ENCRYPTION_ROUTINE_ID << "'" << "\n";
	file_stream << "Encryption Key:        '" << ENCRYPTION_KEY;

	multi_outstream output;
	output.add_stream(file_stream);
	output.add_stream(std::cout);


	//*****************************************************
	// Display and save to the log file the string just input

	output << "\n\nOriginal string  = ";
	output << std::right << std::setw(MAX_CHARS) << std::setfill(' ') << original_chars;

	// output each original char's hex value
	output << " Hex = ";
	for (int i = 0; i < char_count; ++i)
	{
		int const original_char = static_cast <int> (original_chars[i]) & 0xFF; // ANDing with 0xFF prevents static_cast padding 8 bit value with 1s
		output << std::hex << std::right << std::setw(2) << std::setfill('0') << original_char << " ";
	}


	//*****************************************************
	// Encrypt the string and display/save the result

	encrypt_chars(char_count, ENCRYPTION_KEY);

	output << "\n\nEncrypted string = ";
	output << std::right << std::setw(MAX_CHARS) << std::setfill(' ') << encrypted_chars;

	// output each encrypted char's hex value
	output << " Hex = ";
	for (int i = 0; i < char_count; ++i)
	{
		int const encrypted_char = static_cast <int> (encrypted_chars[i]) & 0xFF; // ANDing with 0xFF prevents static_cast padding 8 bit value with 1s
		output << std::hex << std::right << std::setw(2) << std::setfill('0') << encrypted_char << " ";
	}


	//*****************************************************
	// Decrypt the encrypted string and display/save the result

	decrypt_chars(char_count, ENCRYPTION_KEY);

	output << "\n\nDecrypted string = ";
	output << std::right << std::setw(MAX_CHARS) << std::setfill(' ') << decrypted_chars;

	// output each decrypted char's hex value
	output << " Hex = ";
	for (int i = 0; i < char_count; ++i)
	{
		int const decrypted_char = static_cast <int> (decrypted_chars[i]) & 0xFF; // ANDing with 0xFF prevents static_cast padding 8 bit value with 1s
		output << std::hex << std::right << std::setw(2) << std::setfill('0') << decrypted_char << " ";
	}


	//*****************************************************
	// End program

	output << "\n\n";
	file_stream << "-------------------------------------------------------------\n\n";
	file_stream.close();

	system("PAUSE");

	return 0;
}

//**************************************************************************************************************
