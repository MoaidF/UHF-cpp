#include "serialPortCtrl.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>


#define ERR_KEY (-1)




CSerialPortCtrl::CSerialPortCtrl()
{
    m_iFD = ERR_KEY;
}

CSerialPortCtrl::~CSerialPortCtrl()
{
    if (m_iFD != ERR_KEY)
    {
        CloseCom();
        m_iFD = ERR_KEY;
    }
}

void CSerialPortCtrl::setHardwarecontrolFlow(int fd, int on)
{
    struct termios tty;
    tcgetattr(fd, &tty);
    if (on)
        tty.c_cflag |= CRTSCTS;
    else
        tty.c_cflag &= ~CRTSCTS;
    tcsetattr(fd, TCSANOW, &tty);
}

// Set baudrate, parity and number of bits.
void CSerialPortCtrl::setParms(int fd, int baudr, int par, int bit, int stopb, int hwf, int swf)
{
    int spd = -1;
    struct termios tty;
    tcgetattr(fd, &tty);

    switch (baudr)
    {
    case 2400:
        spd = B2400;
        break;
    case 19200:
        spd = B19200;
        break;
    case 38400:
        spd = B38400;
        break;
    case 57600:
        spd = B57600;
        break;
    case 115200:
        spd = B115200;
        break;
    case 9600:
        spd = B9600;
        break;
    default:
        spd = B9600;
        break;
    }

    if (spd != -1)
    {
        cfsetospeed(&tty, (speed_t)spd);
        cfsetispeed(&tty, (speed_t)spd);
    }

    switch (bit)
    {
    case 5:
        tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS5;
        break;
    case 6:
        tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS6;
        break;
    case 7:
        tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS7;
        break;
    case 8:
    default:
        tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
        break;
    }

    /* Set into raw, no echo mode */
    tty.c_iflag = IGNBRK;
    tty.c_lflag = 0;
    tty.c_oflag = 0;
    tty.c_cflag |= CLOCAL | CREAD;

    tty.c_cc[VMIN] = 1;
    tty.c_cc[VTIME] = 5;

    if (swf)
        tty.c_iflag |= IXON | IXOFF;
    else
        tty.c_iflag &= ~(IXON | IXOFF | IXANY);

    tty.c_cflag &= ~(PARENB | PARODD);

    if (par == 1) //输入和输出是奇校验
        tty.c_cflag |= (PARENB | PARODD);
    else if (par == 2) //输入和输出是偶校验
        tty.c_cflag |= PARENB;

    if (stopb == 2)
        tty.c_cflag |= CSTOPB;
    else
        tty.c_cflag &= ~CSTOPB;

    tcsetattr(fd, TCSANOW, &tty);
    setHardwarecontrolFlow(fd, hwf);
}

int CSerialPortCtrl::OpenCom(const char *port, int iBraudrate, int iByteSize, int iParity, int iStopBits, int iContorl)
{
    CMyMutex::CAutoLock lock(m_mutex);

    if (ERR_KEY != m_iFD)
        return E_SERIAL_PORT_SUCCESS;

    int iRet = E_SERIAL_PORT_ERR;
    m_iFD = open(port, O_RDWR | O_NOCTTY | O_NDELAY | O_NONBLOCK);

    // for Android
    if (errno == 2 || m_iFD == -1)
    {
        char buff[256];
        sprintf(buff, "su -c chmod ago+rw ", port);
        system(buff);
        m_iFD = open(port, O_RDWR | O_NOCTTY | O_NONBLOCK);
    } // end for Android

    if (m_iFD < 0)
        return iRet;

    setParms(m_iFD, iBraudrate, iParity, iByteSize, iStopBits, iContorl, iContorl);
    return E_SERIAL_PORT_SUCCESS;
}

int CSerialPortCtrl::CloseCom()
{
    if (m_iFD != ERR_KEY)
    {
        close(m_iFD);
        m_iFD = ERR_KEY;
    }

    return E_SERIAL_PORT_SUCCESS;
}

int CSerialPortCtrl::WriteData(const unsigned char *pData, int &iDataLen, unsigned long ulTimeout, bool isClear)
{
    CMyMutex::CAutoLock lock(m_mutex);

    if (ERR_KEY == m_iFD)
        return E_SERIAL_PORT_ERR;

    int iRet = E_SERIAL_PORT_ERR;
    if (isClear)
        tcflush(m_iFD, TCIOFLUSH); //TCOFLUSH刷新数据但不传送

    int iTotal = 0, iWirteLen, iTryTime = 0;

    do
    {
        iWirteLen = write(m_iFD, pData + iTotal, iDataLen - iTotal); //实际写入的长度
        if ((iWirteLen < 0) && (errno == EAGAIN))
        {
            usleep(1000);
            continue;
        }
        else
        {
            iTotal += iWirteLen;
            if (iTotal == iDataLen)
            {
                iRet = E_SERIAL_PORT_SUCCESS;
                break;
            }
        }
    } while (++iTryTime < ulTimeout);
    iDataLen = iTotal;

    return iRet;
}

int CSerialPortCtrl::WriteDataEx(const unsigned char *pData, int iDataBytes, unsigned long ulTimeout, bool isClear)
{
    CMyMutex::CAutoLock lock(m_mutex);

    if (ERR_KEY == m_iFD)
        return E_SERIAL_PORT_ERR;

    int iRet = E_SERIAL_PORT_ERR;
    if (isClear)
        tcflush(m_iFD, TCIOFLUSH); //TCOFLUSH刷新数据但不传送

    int iTotal = 0, iWirteLen, iTryTime = 0;

    do
    {
        iWirteLen = write(m_iFD, pData + iTotal, iDataBytes - iTotal); //实际写入的长度
        if ((iWirteLen < 0) && (errno == EAGAIN))
        {
            usleep(1000);
            continue;
        }
        else
        {
            iTotal += iWirteLen;
            if (iTotal == iDataBytes)
            {
                iRet = E_SERIAL_PORT_SUCCESS;
                break;
            }
        }
    } while (++iTryTime < ulTimeout);

    return iRet;
}





int CSerialPortCtrl::ReadData(unsigned char *pData, int &iDataBytes, unsigned long ulTimeout)
{
    CMyMutex::CAutoLock lock(m_mutex);

    if (ERR_KEY == m_iFD)
        return E_SERIAL_PORT_ERR;

    int iRet = E_SERIAL_PORT_ERR;

    fd_set fs_read;
    FD_ZERO(&fs_read);
    FD_SET(m_iFD, &fs_read);

    struct timeval tv_timeout;
    struct timeval tv_timeout2;
    tv_timeout.tv_sec = ulTimeout / 1000;
    tv_timeout.tv_usec = (ulTimeout % 1000) * 1000;

    tv_timeout2.tv_sec = 0;
    tv_timeout2.tv_usec = 500000; //0.5 S

    int iTotal = 0, iLen = 0;
    do
    {
        iLen = read(m_iFD, pData + iTotal, iDataBytes - iTotal);
		
		//printf("iLen:%d\r\n",iLen);
		
        if (iLen <= 0)
            break; // buffer data over
        iTotal += iLen;
		
	
		
        if (iTotal >= iDataBytes) // '>'?
        {
			
			
            iRet = E_SERIAL_PORT_SUCCESS;
            return iRet;
        }
    } while (iLen > 0);
    //waite for reading
    if (0 == select(m_iFD + 1, &fs_read, NULL, NULL, &tv_timeout))
    {
        iDataBytes = 0;
        iRet = E_SERIAL_PORT_TIMEOUT;
		
	
    }
    else
    {
        do
        {
            if (FD_ISSET(m_iFD, &fs_read))
            {
                iLen = read(m_iFD, pData + iTotal, iDataBytes - iTotal);
                iTotal += iLen;
				//printf("iLen:%d",iLen);
                if (iTotal >= iDataBytes)
                {
                    iRet = E_SERIAL_PORT_SUCCESS;
                    break;
                }
            }

            if (0 == select(m_iFD + 1, &fs_read, NULL, NULL, &tv_timeout2))
            {
                if (0 == select(m_iFD + 1, &fs_read, NULL, NULL, &tv_timeout))
                {
                    iRet = E_SERIAL_PORT_TIMEOUT;
                    break;
                }
            }
        } while (iLen > 0);
    }
    iDataBytes = iTotal;

    return iRet;
}


extern void BytesToHexStr11111(unsigned char *bBuffer,int bLen,char *strBuf);

int CSerialPortCtrl::ReadDataEx(unsigned char *pData, int &iDataBytes, unsigned long ulTimeout /*= 5000*/)
{
    CMyMutex::CAutoLock lock(m_mutex);

    if (ERR_KEY == m_iFD)
        return E_SERIAL_PORT_ERR;

    int iRet = E_SERIAL_PORT_SUCCESS;
    fd_set fs_read;
    struct timeval tv_timeout;
    struct timeval tv_timeout2;

    FD_ZERO(&fs_read);
    FD_SET(m_iFD, &fs_read);
    tv_timeout.tv_sec = ulTimeout / 1000;
    tv_timeout.tv_usec = (ulTimeout % 1000) * 1000;

    tv_timeout2.tv_sec = 0;
    //tv_timeout2.tv_usec = 500000;
	tv_timeout2.tv_usec = 20000;
    int iTotal = 0, iLen = 0;
	
	char strbuff[1024]={0};
	
    do
    {
        iLen = read(m_iFD, pData + iTotal, iDataBytes - iTotal);
		
		//printf("read len:%d",iLen);
		
		if(iLen>0)
		{
			memset(strbuff,0,sizeof(strbuff));
			BytesToHexStr11111(pData+iTotal,iLen,strbuff);
			//printf("\r\nrecvdata:%s\r\n",strbuff);
		}
		
        if (iLen <= 0)
            break;
        iTotal += iLen;
        if (iTotal >= iDataBytes)
        {
            iRet = E_SERIAL_PORT_SUCCESS;
            return iRet;
        }
    } while (iLen > 0);

    if (0 == select(m_iFD + 1, &fs_read, NULL, NULL, &tv_timeout))
    {
        iDataBytes = 0;
        iRet = E_SERIAL_PORT_TIMEOUT;
    }
    else
    {
        do
        {
            if (FD_ISSET(m_iFD, &fs_read))
            {
                iLen = read(m_iFD, pData + iTotal, iDataBytes - iTotal);
				//printf("\r\nread len:%d\r\n",iLen);
				
				if(iLen>0)
				{
							memset(strbuff,0,sizeof(strbuff));
							BytesToHexStr11111(pData+iTotal,iLen,strbuff);
							//printf("\r\nrecvdata:%s\r\n",strbuff);
				}
				
                iTotal += iLen;
				
            }

            if (0 == select(m_iFD + 1, &fs_read, NULL, NULL, &tv_timeout2))
            {
                break;
            }
        } while (iLen > 0);
    }
    iDataBytes = iTotal;

    return iRet;
}

void CSerialPortCtrl::ClearBuffer()
{
    CMyMutex::CAutoLock lock(m_mutex);

    if (ERR_KEY == m_iFD)
        return;

    tcflush(m_iFD, TCIOFLUSH); //TCOFLUSH刷新数据但不传送
}
