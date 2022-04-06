#ifndef F7602AB5_69D7_4D87_8932_5A97CD7DA553
#define F7602AB5_69D7_4D87_8932_5A97CD7DA553

/**
 * linux serial port communication
 * author: Liu guanglin 
**/

#include <termios.h>
#include <pthread.h>

#define E_SERIAL_PORT_SUCCESS (0)
#define E_SERIAL_PORT_ERR (1)
#define E_SERIAL_PORT_TIMEOUT (2)

class CMyMutex
{
public:
    CMyMutex()
    {
        pthread_mutex_init(&m_mutex, NULL);
    }
    ~CMyMutex()
    {
        pthread_mutex_destroy(&m_mutex);
    }

    void Lock()
    {
        pthread_mutex_lock(&m_mutex);
    }

    void UnLock()
    {
        pthread_mutex_unlock(&m_mutex);
    }

public:
    class CAutoLock
    {
    public:
        CAutoLock(CMyMutex &m_myMutex) : m_lock(m_myMutex)
        {
            m_lock.Lock();
        }
        ~CAutoLock()
        {
            m_lock.UnLock();
        }

    private:
        CAutoLock(CAutoLock &);
        CAutoLock &operator=(CAutoLock &);

    private:
        CMyMutex &m_lock;
    };

private:
    CMyMutex(CMyMutex &);
    CMyMutex &operator=(CMyMutex &);

private:
    pthread_mutex_t m_mutex;
};

class CSerialPortCtrl
{
public:
    CSerialPortCtrl();
    ~CSerialPortCtrl();

public:
    int OpenCom(const char *port, int iBraudrate, int iByteSize = 8, int iParity = 0, int iStopBits = 1, int iContorl = 0);
    int CloseCom();

    int WriteData(const unsigned char *pData, int &iDataBytes, unsigned long ulTimeout = 1000, bool isClear = true);
    int WriteDataEx(const unsigned char *pData, int iDataBytes, unsigned long ulTimeout = 1000, bool isClear = true);

    int ReadData(unsigned char *pData, int &iDataBytes, unsigned long ulTimeout = 5000);
    int ReadDataEx(unsigned char *pData, int &iDataBytes, unsigned long ulTimeout = 5000);

    void ClearBuffer();

private:
    // Set baudrate, parity and number of bits.
    void setParms(int fd, int baudr, int par, int bit, int stopb, int hwf, int swf);
    void setHardwarecontrolFlow(int fd, int on);

private:
    int m_iFD; // io object

private:
    CMyMutex m_mutex;
};

#endif // F7602AB5_69D7_4D87_8932_5A97CD7DA553
