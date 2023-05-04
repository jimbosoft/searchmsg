/* <T> KWTAB Key Word Table Definitions                              */
/**********************************************************************
   
    HEADER 	kwtab.h
   
    DESCRIPTION	structure definitions for Key Word Table
   
   
    $Header :$

    HISTORY 
    v1.50 22/08/14  T Crawford  add NOTAM 
    v1.49 08/05/14  T Crawford  add CHART 
    v1.48 14/04/10  T Crawford  add AFIRS
    v1.47 16/02/10  T Crawford  remove RQ 79
    v1.46 04/06/07  T Crawford  add GPATS
    v1.45 07/03/07  I Senior    add DART buoy
    v1.44 17/11/06  T Crawford	add AERODROME for type 81
    v1.43 04/09/06  T Crawford	add BUFR
    v1.42 17/08/06  T Crawford	add OMDQ8
          19/12/05  T Crawford	replace GRAF with NVP
    v1.41 07/07/05  T Crawford	replace WINTEM with CEVI
    v1.40 08/10/04  T Crawford	replace NACLI with CREX
    v1.40 08/10/04  T Crawford	replace IAC with XMD
    v1.39 14/12/01  T Crawford	replace LLXX with ATIS
                                add FLT and FLIGHT for FIFOR
    v1.39 24/01/01  Ian Senior  Add TENMD (10 minute data). Remove ICEAN
    v1.38 10/12/00  T Crawford  add GRIB
    v1.37 15/02/00  T Crawford  add IAC FLEET and NACLI etc
    v1.36 15/02/00  T Crawford  change fm to char
    v1.35 09/09/99  T Crawford  add PROV TAF
    v1.34 24/06/99  T Crawford  remove DISCON
    v1.33 17/03/99  T Crawford  add HYBRID
    v1.32 11/08/98  T Crawford  add function prototypes for kwtab.oc
    v1.31 06/08/98  T Crawford  add MOBIL types
    v1.30 26/05/98  T Crawford  add wmo_inc
    v1.29 06/10/97  T Crawford  add EEBB
    v1.28 25/07/96  T Crawford  fix array size for tab

    v1.0  15/07/93  T Crawford  original code
    v1.1  12/11/93  T Crawford  add codename and fm
    v1.2  24/11/93  T Crawford  add #defines for keywords
    v1.3  14/02/94  T Crawford  add #define for ACARS
    v1.4  16/03/94  T Crawford  add #define for AMDAR
    v1.5  01/06/94  T Crawford  add #define for CLIMAT, CLIMAT_TEMP
    v1.6  17/06/94  I Senior    add #define for DISCON
    v1.7  07/07/94  T Crawford  add #define for CLIMAT_SHIP & TEMP_SHIP
    v1.8  29/08/94  I Senior    add #define for UUAA..BB, QQAA..BB, ZZXX
    v1.9  03/02/95  T Crawford  add #define for SIGMET, AIRMET, TEXT
    v1.10 07/03/95  T Crawford  add #define for ARFOR, AREA
    v1.11 28/03/95  T Crawford  add #define for TAF
    v1.12 06/04/95  T Crawford  add #define for all keywords
    v1.13 18/04/95  T Crawford  add #define for ROUTE & GRID
    v1.14 27/04/95  T Crawford  change TEXT to 98 / 99 = all msgtypes
    v1.15 24/07/95  T Crawford  comments cc compatible
    v1.16 30/08/95  T Crawford  add separate msgtype for AWS reports
                                add stationtype field
                                add #define values for stationtype
    v1.17 06/09/95  T Crawford  add #define for DFXX
    v1.18 26/09/95  T Crawford  add #define for VOLCANIC, JJYY
    v1.19 16/10/95  T Crawford  add #define for OneMinuteData
    v1.20 17/01/96  T Crawford  add #define KW_LIMIT for last entry
    v1.21 17/01/96  T Crawford  add #define for UNKNOWN report type
                                change KW_LIMIT from 0 to -1
    v1.22 07/02/96  I Senior	add typedef struct defns (AIFS/IBM)
    v1.23 23/05/96  T Crawford  add #define for MOS GUIDANCE
    v1.24 05/09/96  I Senior	add AVOBS type - for Aviation Obs.
**********************************************************************/
#ifndef _KWTAB_H
#define _KWTAB_H

#define MAX_KEYWORDS 200
#define MAX_KEYWORD_LENGTH 20

#define _UNDEF	0
#define _FIXED	1
#define _MOBILE	2
#define _HYBRID	3

#define	KW_LIMIT	-1

#define AVOBS	990
#define UNKNOWN	100
#define	AAXX	1
#define	PPAA	2
#define	PPBB	3
#define	PPCC	4
#define	PPDD	5
#define TTAA	6
#define TTBB	7
#define TTCC	8
#define TTDD	9
#define SPECI	10
#define METAR	11
#define	BBXX	12
#define TTF	13
#define QQAA	14
#define QQBB	15
#define QQCC	16
#define QQDD	17
#define UUAA	18
#define UUBB	19
#define UUCC	20
#define UUDD	21
#define AIREP_SPECIAL	22
#define AIREP	23
#define IIAA	24
#define KKXX	25
#define GTSMSG  26
#define	ACARS	27
#define	AMDAR	28
#define	RAREP	29
#define	ROFOR	30
#define	TAFOR	31
#define	TAF	31
#define	PROV	31
#define	N2T	32
#define IIBB	33
#define	FFAA	34
#define	FFBB	35
#define	GGAA	36
#define	GGBB	37
#define	OOXX	38
#define	XXAA	39
#define	XXBB	40
#define	XXCC	41
#define	XXDD	42
#define	IICC	43
#define	IIDD	44
#define	ATIS	45
#define	NNXX	46
#define	MMXX	47
#define	HHXX	48
#define	CCAA	49
#define	CCBB	50
#define	DDAA	51
#define	DDBB	52
#define	VVAA	53
#define	VVBB	54
#define	VVCC	55
#define	VVDD	56
#define	WWXX	57
#define	YYXX	58
#define	TENMD	59
#define	NVP	60
#define	CEVI	61
#define	ARFOR	62
#define	MAFOR	63
#define	HYFOR	64
#define	CLIMAT			65
#define	CLIMAT_SHIP		66
#define	CLIMAT_TEMP		67
#define	CLIMAT_TEMP_SHIP	68
#define	EEAA	69
#define	EECC	70
#define	EEDD	71
#define	GRIB	72
#define	ZZXX	73
#define	SIGMET	74
#define	AIRMET	75
#define	AREA	76
#define	AREA_QNH	77
#define	PROG	78
#define	REQUEST	79
#define	REQ	79
#define	FIFOR	80
#define	FLT	80
#define	FLIGHT	80
#define	AIRPORT	81
#define	AD	81
#define	AERODROME	81
#define	MARITIME	82
#define	HHRR	83
#define	HHAA	84
#define	HHZZ	85
#define	ROUTE	86
#define	GRID	87
#define SPECIAWS	88
#define METARAWS	89
#define	DFXX	90
#define VOLCANIC	91
#define JJXX	92
#define MOS	93
#define EEBB	94
#define XMD	95
#define CREX	96
#define	OMD	97
#define	TEXT	98
#define	OMDQ8	111
#define	DARTBUOY 112
#define	BUFR	121
#define KKYY	125
#define	GPATS	131
#define	AMDAR3	141
#define	AFIRS 	142
#define	CHART 	143
#define	NOTAM 	144

typedef struct KWITEM {
   int msgtype;
   int stationtype;
   char keyword [MAX_KEYWORD_LENGTH+1];
   char codename [MAX_KEYWORD_LENGTH+1];
   char fm [MAX_KEYWORD_LENGTH+1];
   int wmo_inc;     
   } KWITEM;

typedef struct KW {
   KWITEM tab[MAX_KEYWORDS+1];
   } KW;

#ifdef __cplusplus
extern "C" {
#endif

int create_kwtab_table (int argc, char *argv []);
int populate_kwtab_table (void);

#ifdef __cplusplus
};
#endif

#endif
