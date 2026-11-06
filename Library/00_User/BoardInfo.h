#ifndef	_BOARDINFO_H
#define	_BOARDINFO_H

#include	"UserDefine.h"

typedef	struct{
	u16	mode;
	u8	mac[6];
	u8	ip[4];
	u8	nm[4];
	u8	gw[4];
	u16	tcpPort;
	u16	udpPort;
}networkInfo_t;

#endif	//	_BOARDINFO_H
