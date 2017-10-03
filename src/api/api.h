#pragma once

#include <cstdint>

namespace kiv_os {


using THandle = uint16_t;  //16-bitu, aby se zabranilo predavani instanci trid mezi jadrem a procesem 
						   //namisto integerovych handlu jako v realnem OS


struct TGeneral_Register {
	union {
		uint64_t r;			//e.g., rax
		uint32_t e;			//e.g., eax
		uint16_t x;			//e.g.; ax
		struct {
			uint8_t l;		//e.g., al
			uint8_t h;		//e.g., ah
		};
	};
};

struct TIndex_Register {
	union {
		uint64_t r;			//e.g., rdi
		uint32_t e;			//e.g., edi
		uint16_t i;			//e.g.; di
	};
};



struct TFlags {
	uint8_t carry : 1;
};

struct TRegisters {
	TGeneral_Register rax, rcx, rdx;
	TIndex_Register rdi;
	TFlags flags;
};	//vice registru neni treba


using TEntry_Point = size_t(__stdcall *)(const TRegisters context);		//vstupni bod uzivatelskeho programu
using TSys_Call = void(__stdcall *)(TRegisters &context);			//prototyp funkce, ktera realizuje syscall
using TThread_Proc = size_t(__stdcall *)(const void *data);			//vstupni bod programu vlakna


struct TDir_Entry {
	uint16_t file_attributes;			//viz konstanty fa
	char file_name[8 + 1 + 3];	//8.3 FAT
};

struct TProcess_Startup_Info {
	char *arg;			//pointer na parametry
	THandle stdin, stdout, stderr;	//kazdy, ktery bude erInvalid_Handle, bude mit vychozi hodnotu
};

/*

   Cisla funkci OS:
	 AH - major cislo aka id skupiny fci
	 AL - minor cislo aka cisle konkretni fce

	 je-li po volani nastavena vlajka carry, tj. TRegisters.flags.carry != 0, pak Rax je kod chyby

	  AH == 1 : IO operace
		AL: cislo IO operace	//konzole je take jenom soubor
			1 - otevrit soubor				 IN: rdx je pointer na null-terminated ANSI char string udavajici file_name;
												 rcx jsou flags k otevreni souboru - viz fm konstanty
												 rdi jsou atributy vytvareneho souboru
											OUT: ax je handle nove otevreneho souboru

			2 - zapis do souboru			 IN: dx je handle souboru, rdi je pointer na buffer, rcx je pocet bytu v bufferu k zapsani
											OUT: rax je pocet zapsanych bytu
			3 - cti ze souboru
											 IN: dx je handle souboru, rdi je pointer na buffer, kam zapsat, rcx je velikost bufferu v bytech
											OUT: rax je pocet prectenych bytu
			4 - nastav pozici v souboru		 IN: dx je handle souboru, rdi je nova pozice v souboru
												cl konstatna je typ pozice (jedna z fs konstant),
												ch == 0 jenom nastavn pozici (fsSet_Position)
												ch == 1 nastav pozici a nastav velikost souboru na tuto pozici (fsSet_Size)

			5 - cti  pozici v souboru		 IN: dx je handle souboru, rcx je typ pozice (jedna z fs konstant),
											 OUT: rax je pozice v souboru
								//u AL == 5 | 6 plati ze rcx je
											fsBeginning: od zacatku souboru
											fsCurrent: od aktualni pozice v souboru
											fsEnd: od konce souboru

			6 - zavri handle			 IN: dx  je handle libovolneho typu k zavreni

			7 - ziskej pracovni adresar		IN: rdx je pointer na ANSI char buffer, rcx je velikost buffer
										   OUT: rax pocet zapsanych znaku

			8 - nastav pracovni adresar    IN: rdx je pointer na null-terminated ANSI char string udavajici novy adresar (muze byt relativni cesta)

			9 - vytvore pipe				IN: rdx je pointer na pole dvou Thandle - prvni zapis a druhy pro cteni z pipy

			vytvoreni adresare - vytvori se soubor s atributem adresar
			smazani adresare - smaze se soubor
			vypis adresare - otevre se adresar jako read-only soubor a cte se jako binarni soubor obsahujici jenom polozky TDir_Entry


	  AH == 2 : Proc operace
		AL: cislo IO operace	//konzole je take jenom soubor
			1 - Clone			IN:	rcx je cl konstanta
										clCreateProcess a pak rdx je je pointer na null-terminated string udavajici jmeno souboru ke spusteni (tj. retezec pro GetProcAddress v kernelu)
														rdi je pointer na TProcess_Startup_Info
									anebo
									   clCreateThread a pak rdx je TThreadProc a rdi jsou *data
									OUT: rax je handle noveho procesu/threadu

										OUT: ax je handle nove vytvoreneho procesu
			2 - cekej na handle			 IN: rdx pointer na pole THandle, na ktere se ma cekat, rcx je pocet handlu
										OUT: rax je index handle, ktery byl signalizovan




   Dalsi cisla si doplnte dle potreby, ovsem musite je dukladne zduvodnit, proc to bez nich nejde a dat to do prubezne prezentace,
   finalni dokumentaci a api_extension.h.

   Co nemuzete zjistit z API, to zkuste nejprve vyresit pomoci souboroveho systemu, napr. adresarem 0:\procfs

*/



//Cisla systemovych sluzeb - ah je typ operace, al je konkretni operace

//ah hodnoty 
const uint8_t scIO = 1;		//IO operace

							//al hodnoty pro scIO 
const uint8_t scCreate_File = 1;
const uint8_t scWrite_File = 2;
const uint8_t scRead_File = 3;
const uint8_t scSet_File_Position = 4;
const uint8_t scGet_File_Position = 5;
const uint8_t scClose_Handle = 6;
const uint8_t scGet_Current_Directory = 7;
const uint8_t scSet_Current_Directory = 8;
const uint8_t scCreate_Pipe = 9;


const uint8_t scProc = 2;	//sprava procesu a vlaken
							//al hodnoty pro scProc
const uint8_t scClone = 1;
const uint8_t scWait_For = 2;


//Navratove kody OS

const uint16_t erSuccess = 0;										//vse v poradku
const uint16_t erInvalid_Handle = static_cast<uint16_t>(-1);		//neplatny handle 
const uint16_t erInvalid_Argument = 2;								//neplatna kombinace vstupnich argumentu
const uint16_t erFile_Not_Found = 3;								//soubor  nenalezen

//atributy souboru
const uint8_t faRead_Only = 0x01;
const uint8_t faHidden = 0x02;
const uint8_t faSystem_File = 0x04;
const uint8_t faVolume_ID = 0x08;
const uint8_t faDirectory = 0x10;
const uint8_t faArchive = 0x20;

//hodnoty typu nastaveni pozice
const uint8_t fsBeginning = 0;
const uint8_t fsCurrent = 1;
const uint8_t fsEnd = 2;

const uint8_t fsSet_Position = 0;
const uint8_t fsSet_Size = 1;


//konstanty pro volani clone
const uint8_t clCreate_Process = 1;
const uint8_t clCreate_Thread = 2;

//rezim otevreni noveho souboru
const uint8_t fmOpen_Always = 1;	//pokud je nastavena, pak soubor musi existovat, aby byl otevren
									//není-li fmOpen_Always nastaveno, pak je soubor vždy vytvoøen - tj. i pøepsán starý soubor

//id standardnich i/o
const THandle stdInput = 1;
const THandle stdOutput = 2;
const THandle stdError = 3;



#include "api_extension.h"

}