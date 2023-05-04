/* <T> KWTABDEF Key Word Table Definitions  */
/********************************************************************
   
    DEFINITION  kwtabdef.h
   
    DESCRIPTION	definition of valid keywords and message types
  				  kwtab.h contains header defines etc.
   
    $Header: /home/commsa/cmdv/cmss/src/RCS/kwtabdef.h,v 1.86 2015/03/17 03:57:51 tyc Exp $

    HISTORY 

    v1.79 22/08/14  T Crawford	add NOTAM
    v1.78 18/08/10  T Crawford	change NNXX to _MOBILE
    v1.77 19/04/10  T Crawford	add AFIRS
    v1...
    v1.74 15/02/10  T Crawford	remove RQ
    v1.73 04/06/07  T Crawford	add GPATS
    v1.72 13/02/07  T Crawford	add DARTBUOY
    v1.71 17/11/06  T Crawford	add AERODROME for type 81
    v1.70 04/09/06  T Crawford	add BUFR
    v1.69 17/08/06  T Crawford	add OMDQ8
          19/12/05  T Crawford	replace GRAF with NVP
    v1.68 17/08/05  T Crawford	replace WINTEM with CEVI
    v1.67 21/07/05  T Crawford	change AREA QNH TO _FIXED
    v1.66 04/03/05  T Crawford	remove ADS AMDAR keyword
    v1.65 14/02/05  T Crawford	remove NACLI add CREX
    v1.64 15/12/04  T Crawford	add ADS AMDAR keyword
    v1.63 08/10/04  T Crawford	replace IAC with XMD
    v1.62 09/06/04  T Crawford	add ASHTAM synonym for VOLCANIC
    v1.61 17/11/03  T Crawford	MMXX = HYBRID
    v1.60 06/11/03  T Crawford	MMXX = FIXED
    v1.59 12/12/01  T Crawford	replace LLXX with ATIS
                                add FLT FLIGHT alternatives for FIFOR
    v1.58 20/02/01  T Crawford	add OMD and TMD
    v1.57 24/01/01  Ian Senior	Remove ICEAN. 
    v1.55 02/01/01  T Crawford	move PROV TAF again
    v1.54 10/12/00  T Crawford	add GRIB
    v1.53 29/03/00  T Crawford	add IAC FLEET and NACLI etc
    v1.52 02/03/00  T Crawford	change to CLIMAT keywords 
    v1.51 16/02/00  T Crawford	change fm to char 
    v1.50 09/02/00  T Crawford	remove NULL references 
    v1.49 15/12/99  T Crawford	add RQST
    v1.48 14/10/99  T Crawford	move PROV TAF
    v1.47 09/09/99  T Crawford	add PROV TAF
    v1.46 24/06/99  T Crawford	remove DISCON
    v1.45 24/06/99  T Crawford	add alternative VOLCANIC keywords
    v1.44 17/03/99  T Crawford	MMXX = HYBRID
    v1.43 16/12/98  T Crawford	MMXX = FIXED
    v1.42 11/08/98  T Crawford	fix TEXT
    v1.41 11/08/98  T Crawford	one more
    v1.40 11/08/98  T Crawford  small changes
    v1.39 06/08/98  T Crawford  add MOBIL types
    v1.38 26/05/98  T Crawford  add wmo_inc
    v1.37 02/04/98  T Crawford  add POS and AEP FOR AIREP
    v1.36 28/10/97  T Crawford  add CLIMATE, KLIMAT as alternative
    v1.35 06/10/97  T Crawford  add EEBB
    v1.34 25/09/97  T Crawford  add AIREPS as alternative

    v1.32 30/07/97  T Crawford  change ZZYY to _MOBILE

    v1.1  15/07/93  T Crawford  Original code	        
    v1.2  27/07/93  T Crawford  Remove space in front of keywords
                                Change name to KWTABDEF.H
    v1.3  14/10/93  T Crawford  Remove spaces from keywords
  				Correct msgtypes for METARs/SPECIs 
    v1.4  12/11/93  T Crawford  Add other WMO codes
                                Include CODE NAME and FM number
   				CODE NAME is blank if it is the same
  				as the keyword
  				FM is 0 for non-WMO codes
    v1.5  27/07/94  T Crawford  fix CLIMAT SHIP keyword
    v1.6  22/08/94  T Crawford  add AMDAR keyword        
    v1.7  25/10/94  T Crawford  add DRIFTER code ZZYY keyword        
                                add SIGMET keyword
    v1.8  01/02/95  T Crawford  add TEXT keyword
    v1.9  03/02/95  T Crawford  add AIRMET keyword
    v1.10 27/02/95  T Crawford  add PROG keyword
    v1.11 01/03/95  T Crawford  add REQUEST keyword
                                add FIFOR keyword
                                add MARITIME keyword
                                add AIRPORT keyword
                                add TAFOR keyword
    v1.12 29/03/95  T Crawford  add HHRR keyword
    v1.13 06/04/95  T Crawford  add HHAA and HHXX keywords
    v1.14 18/04/95  T Crawford  add ROUTE SECTOR / GRID POINT keywords
    v1.15 24/07/95  T Crawford  comments cc compatible
    v1.16 30/08/95  T Crawford  add separate msgtypes for AWS reports
                                add stationtype field
    v1.17 26/09/95  T Crawford  add VOLCANIC JJYY keywords
    v1.18 17/01/96  T Crawford  change last entry to use LIMIT
    v1.19 17/01/96  T Crawford  add UNKNOWN report type
    v1.20 19/01/96  T Crawford  add exta GRID type
    v1.21 23/05/96  T Crawford  add MOS GUIDANCE keyword
*********************************************************************/
#ifndef	_KWTABDEF
#define	_KWTABDEF

#include 	"kwtab.h"

/*
LLXX,		_UNDEF,	"LLXX",	"CODAR",		"41-IV",	0,
*/

static KW kwtab = {

/*
CMSS		Station	KEYWORD	CODE NAME		FM
msgtype		Type						*/

UNKNOWN,	_UNDEF,	"",	"UNKNOWN",		"",	0,

AAXX,		_FIXED,	"AAXX",	"SYNOP",		"12-XI",	0,
PPAA,		_FIXED,	"PPAA",	"PILOT PART A",		"32-IX",	0,
PPBB,		_FIXED,	"PPBB",	"PILOT PART B",		"32-IX",	0,
PPCC,		_FIXED,	"PPCC",	"PILOT PART C",		"32-IX",	0,
PPDD,		_FIXED,	"PPDD",	"PILOT PART D",		"32-IX",	0,
TTAA,		_FIXED,	"TTAA",	"TEMP PART A",		"35-X Ext.",	0,
TTBB,		_FIXED,	"TTBB",	"TEMP PART B",		"35-X Ext.",	0,
TTCC,		_FIXED,	"TTCC",	"TEMP PART C",		"35-X Ext.",	0,
TTDD,		_FIXED,	"TTDD",	"TEMP PART D",		"35-X Ext.",	0,

BBXX,		_MOBILE,"BBXX",	"SHIP",			"13-XI",	0,
QQAA,		_MOBILE,"QQAA",	"PILOT SHIP PART A",	"33-IX",	0,
QQBB,		_MOBILE,"QQBB",	"PILOT SHIP PART B",	"33-IX",	0,
QQCC,		_MOBILE,"QQCC",	"PILOT SHIP PART C",	"33-IX",	0,
QQDD,		_MOBILE,"QQDD",	"PILOT SHIP PART D",	"33-IX",	0,
UUAA,		_MOBILE,"UUAA",	"TEMP SHIP PART A",	"36-X Ext.",	0,
UUBB,		_MOBILE,"UUBB",	"TEMP SHIP PART B",	"36-X Ext.",	0,
UUCC,		_MOBILE,"UUCC",	"TEMP SHIP PART C",	"36-X Ext.",	0,
UUDD,		_MOBILE,"UUDD",	"TEMP SHIP PART D",	"36-X Ext.",	0,

SPECIAWS,	_FIXED,	"SPECIAWS","",			"",	0,
SPECI,		_FIXED,	"SPECI", "",			"16-X Ext.",	0,
METARAWS,	_FIXED,	"METARAWS","",			"",	0,
METAR,		_FIXED,	"METAR","",			"15-X Ext.",	0,
TTF,		_FIXED,	"TTF",	"TTF METAR/SPECI",	"15-X Ext.",	0,

AIREP_SPECIAL,	_MOBILE,"ARS",	"AIREP SPECIAL",	"",	0,
AIREP,		_MOBILE,"ARP",	"AIREP",		"",	0,
AIREP,		_MOBILE,"AEP",	"AIREP",		"",	0,
AIREP,		_MOBILE,"POS",	"AIREP",		"",	0,
AIREP,		_MOBILE,"AIREPS","AIREP",		"",	0,
AIREP,		_MOBILE,"AIREP","AIREP",		"",	0,

ACARS,		_MOBILE,"WXO",	"ACARS",		"",	0,
AMDAR,		_MOBILE,"AMDAR","AMDAR",		"42-XI",	0,
AMDAR,		_MOBILE,"DFD",	"AMDAR",		"42-XI",	0,
AMDAR,		_MOBILE,"ADS",	"AMDAR",		"42-XI",	0,
N2T,		_MOBILE,"N2T",	"AMDAR",		"42-XI",	0,
AMDAR3,		_MOBILE,"AMDAR3","AMDAR3",		"42-XI",	0,
AFIRS,		_MOBILE,"AFIRS","",			"",	0,

TAF,		_FIXED,	"TAF",	"",			"51-X Ext.",	0,
TAFOR,		_FIXED,	"TAFOR","",			"51-X Ext.",	0,
PROV,		_FIXED,	"PROV",	"PROV TAF",		"",	0,

RAREP,		_UNDEF,	"RAREP","",			"",	0,
ROFOR,		_FIXED,	"ROFOR","",			"54-X Ext.",	0,
ARFOR,		_FIXED,	"ARFOR","",			"53-X Ext.",	0,
MAFOR,		_FIXED,	"MAFOR","",			"61-IV",	0,
HYFOR,		_FIXED,	"HYFOR","",			"68-VI",	0,

JJXX,		_MOBILE,"JJXX",	"BATHY",		"63-X Ext.",	0,
JJXX,		_MOBILE,"JJYY",	"BATHY",		"63-X Ext.",	0,
JJXX,		_MOBILE,"JJVV",	"BATHY",		"63-X Ext.",	0,
KKXX,		_MOBILE,"KKXX",	"TESAC",		"64-XI Ext.",	0,
KKXX,		_MOBILE,"KKYY",	"TESAC",		"64-XI Ext.",	0,
KKYY,		_MOBILE,"KKYY",	"TESAC",		"64-IX",	0,
ZZXX,		_MOBILE,"ZZXX",	"BUOY",			"18-XI",	0,
ZZXX,		_MOBILE,"ZZYY",	"BUOY",			"18-XI",	0,
MMXX,		_HYBRID,"MMXX",	"WAVEOB",		"65-XI",	0,
NNXX,		_MOBILE,"NNXX",	"TRACKOB",		"62-VIII Ext.",	0,
HHXX,		_UNDEF,	"HHXX",	"HYDRA",		"67-VI",	0,

CLIMAT,		_FIXED,	"CLIMAT","",			"71-XI",	0,
CLIMAT,		_FIXED,	"KLIMAT","CLIMAT",		"71-XI",	0,
CLIMAT_SHIP,	_MOBILE,"CLIMAT SHIP","",		"72-VI",	0,
CLIMAT_TEMP,	_FIXED,	"CLIMAT TEMP","",		"75-X",	0,
CLIMAT_TEMP_SHIP,_MOBILE,"CLIMAT TEMP SHIP","",		"76-X",	0,

OOXX,		_MOBILE,"OOXX",	"SYNOP MOBIL",		"14-XI",	0,
EEAA,		_MOBILE,"EEAA",	"PILOT MOBIL PART A",	"34-IX",	0,
EEBB,		_MOBILE,"EEBB",	"PILOT MOBIL PART B",	"34-IX",	0,
EECC,		_MOBILE,"EECC",	"PILOT MOBIL PART C",	"34-IX",	0,
EEDD,		_MOBILE,"EEDD",	"PILOT MOBIL PART D",	"34-IX",	0,
IIAA,		_MOBILE,"IIAA",	"TEMP MOBIL PART A",	"38-X Ext.",	0,
IIBB,		_MOBILE,"IIBB",	"TEMP MOBIL PART B",	"38-X Ext.",	0,
IICC,		_MOBILE,"IICC",	"TEMP MOBIL PART C",	"38-X Ext.",	0,
IIDD,		_MOBILE,"IIDD",	"TEMP MOBIL PART D",	"38-X Ext.",	0,
XXAA,		_UNDEF,	"XXAA",	"TEMP DROP PART A",	"37-X Ext.",	0,
XXBB,		_UNDEF,	"XXBB",	"TEMP DROP PART B",	"37-X Ext.",	0,
XXCC,		_UNDEF,	"XXCC",	"TEMP DROP PART C",	"37-X Ext.",	0,
XXDD,		_UNDEF,	"XXDD",	"TEMP DROP PART D",	"37-X Ext.",	0,

SIGMET,		_FIXED,	"SIGMET","",			"",	0,
AIRMET,		_FIXED,	"AIRMET","",			"",	0,
AREA,		_UNDEF,	"AREA",	"",			"",	0,
AREA_QNH,	_FIXED,	"AREA QNH","",			"",	0,
PROG,		_UNDEF,	"PROG",	"",			"",	0,
REQUEST,	_UNDEF,	"REQUEST","",			"",	0,
REQUEST,	_UNDEF,	"RQST",	"",			"",	0,
REQ,		_UNDEF,	"REQ",	"",			"",	0,
FIFOR,		_FIXED,	"FIFOR","",			"",	0,
FIFOR,		_FIXED,	"FLT","",			"",	0,
FIFOR,		_FIXED,	"FLIGHT","",			"",	0,
AIRPORT,	_FIXED,	"AIRPORT","",			"",	0,
AD,		_FIXED,	"AD","",			"",	0,
AERODROME,	_FIXED,	"AERODROME","",			"",	0,
MARITIME,	_FIXED,	"MARITIME","",			"",	0,

HHRR,		_FIXED,	"HHRR",	"HYDATA",		"",	0,
HHAA,		_FIXED,	"HHAA",	"HYALARM",		"",	0,
HHZZ,		_FIXED,	"HHZZ",	"HYREP",		"",	0,

GRID,		_FIXED,	"GFAU",	"GRID PT",		"",	0,
GRIB,		_UNDEF,	"GRIB",			"","92-X Ext.",	0,

BUFR,		_UNDEF,	"BUFR",	"",		"94-XIII",	0,
AAXX,		_FIXED,	"BUFR",	"BUFR",		"94-XIII",	0,
PPAA,		_FIXED,	"BUFR",	"BUFR",		"94-XIII",	0,
PPBB,		_FIXED,	"BUFR",	"BUFR",		"94-XIII",	0,
PPCC,		_FIXED,	"BUFR",	"BUFR",		"94-XIII",	0,
PPDD,		_FIXED,	"BUFR",	"BUFR",		"94-XIII",	0,
TTAA,		_FIXED,	"BUFR",	"BUFR",		"94-XIII",	0,
TTBB,		_FIXED,	"BUFR",	"BUFR",		"94-XIII",	0,
TTCC,		_FIXED,	"BUFR",	"BUFR",		"94-XIII",	0,
TTDD,		_FIXED,	"BUFR",	"BUFR",		"94-XIII",	0,
CLIMAT,		_FIXED,	"BUFR",	"BUFR",		"94-XIII",	0,
MMXX,		_HYBRID,"BUFR",	"BUFR",		"94-XIII",	0,
CREX,		_FIXED,	"BUFR",	"BUFR",		"94-XIII",	0,
JJXX,		_MOBILE,"BUFR",	"BUFR",		"63-X Ext.",	0,

CREX,		_FIXED,	"CREX",	"",		"95-XIII",	0,
ROUTE,		_FIXED,	"FBAU",	"ROUTE",		"",	0,

OMD,		_FIXED,	"OMD","",			"",	0,
OMDQ8,		_FIXED,	"iii","OMD AWOS",		"",	0,
TENMD,		_FIXED,	"TMD","",			"",	0,
XMD,		_FIXED,	"XMD","",			"",	0,

DARTBUOY,	_FIXED, "DARTBUOY","",		"SPECIAL CODE",	0,

ATIS,		_FIXED,	"ATIS",	"",			"",	0,
CEVI,		_FIXED,	"CEVI","",			"",	0,
DFXX,		_FIXED,	"DFXX",	"MDF",			"",	0,
MOS,		_UNDEF,	"MOS","",			"",	0,
NVP,		_FIXED,	"NVP",	"",			"",	0,

VOLCANIC,	_UNDEF,	"VOLCANIC","",			"",	0,
VOLCANIC,	_UNDEF,	"VOLCANO","",			"",	0,
VOLCANIC,	_UNDEF,	"VOLC","",			"",	0,
VOLCANIC,	_UNDEF,	"ASHTAM","",			"",	0,

GPATS,		_UNDEF,	"GPATS","",			"",	0,

FFAA,		_UNDEF,	"FFAA",	"RADOB PART A",		"20-VIII",	0,
FFBB,		_UNDEF,	"FFBB",	"RADOB PART B",		"20-VIII",	0,
GGAA,		_UNDEF,	"GGAA",	"RADOB SHIP PART A",	"20-VIII",	0,
GGBB,		_UNDEF,	"GGBB",	"RADOB SHIP PART B",	"20-VIII",	0,
CCAA,		_UNDEF,	"CCAA",	"SAREP PART A",		"85-IX",	0,
CCBB,		_UNDEF,	"CCBB",	"SAREP PART B",		"85-IX",	0,
DDAA,		_UNDEF,	"DDAA",	"SAREP SHIP PART A",	"85-IX",	0,
DDBB,		_UNDEF,	"DDBB",	"SAREP SHIP PART B",	"85-IX",	0,
VVAA,		_UNDEF,	"VVAA",	"SATEM PART A",		"86-XI",	0,
VVBB,		_UNDEF,	"VVBB",	"SATEM PART B",		"86-XI",	0,
VVCC,		_UNDEF,	"VVCC",	"SATEM PART C",		"86-XI",	0,
VVDD,		_UNDEF,	"VVDD",	"SATEM PART D",		"86-XI",	0,
WWXX,		_UNDEF,	"WWXX",	"SARAD",		"87-XI",	0,
YYXX,		_UNDEF,	"YYXX",	"SATOB",		"88-XI",	0,

CHART,		_UNDEF,	"CHART","",			"",	0,
NOTAM,		_UNDEF,	"NOTAM","",			"",	0,
TEXT,		_FIXED,	"TEXT","TEXT",			"",	0,
KW_LIMIT,0,"","","",0
};
#endif
