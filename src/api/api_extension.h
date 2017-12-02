#pragma once

//sem se dopise, co vam chybelo v api.h - zduvodni se to zde, v prubezne prezentaci a finalni dokumentaci
//soubor api.h se nebude menit


const uint16_t erProces_Not_Created = 4;								// proces nebyl vytvoren z duvodu nedostatku prostredku

// Dodatecne syscally

// IO

/*
IN:		rdx.x - pointer

OUT:	rax.l - file attributes according to api.h
*/
const uint8_t scGetFileAttributes = 11;

// proc
const uint8_t scShutdown = 3;									// ukonci vsechny bezici procesy a vypni PC
