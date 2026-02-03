/******************************************************************************
 * @file 		TChronoMeter
 * @brief		This class is used to configure and use the Serial ports on
 * @author		IVAN Perletti - General Medical Merate.spa - Seriate - ITALY
 * @version		1.0
 * @date		April 16th, 2019
 * @pre			Initialize and Enable the Serial class for the communication
 * @post 		Nope
 * @bug			Not all memory is freed when deleting an object of this class.
 * @warning		Improper use can crash your application
 * @copyright 	GMM.spa - All Rights Reserved
 *
 ******************************************************************************/
#ifndef TCHRONOMETER_H
#define TCHRONOMETER_H

#include <QTimer>


class TClock
{
private:
    QTimer *timer;
    int iTimerId; /*! timer Id for priodical interrupt*/
public:
    unsigned long ulTimeCntr;

public:
    void IRQ_update (long lDelta_ms);
    static TClock& instance()
    {
        static TClock tClock;
        return tClock;
    }
    inline unsigned long watch(void){return (ulTimeCntr);}

private:
    TClock();
};
extern TClock tClock;

class TChronoMeter
{
private:
    TClock* universalClock;
    unsigned long u32Tick0;
    unsigned long u32Tick1;
    bool bPaused;
    float fTicPrescaler; /// prescaler for the TChronometer

public:
    TChronoMeter();
    virtual ~TChronoMeter();
    void start(void);
    void pause(void);
    void resume(void);
    float watch(void);
    //	bool exceed(long lTimeThreshold);
    bool exceed(float lTimeThreshold);
    /**
     * @brief It adds positive or negative Time Delay
     * @param lDeltaTime 	amount of time to be shifted
     */
    inline void shift(const float fDeltaTime){u32Tick0-=fDeltaTime*fTicPrescaler;}
    void stopReset(void);
    float now(void);
};

#endif // TCHRONOMETER_H
