/******************************************************************************
 * @file 		TChronoMeter class
 * @brief		This class is used to configure and use the Timer ports on
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

#include "TChronoMeter.h"

#include <QTimerEvent>

TClock tClock = TClock::instance();
//------------------------------------------------------------------------------
TClock::TClock()
{
    // timer = new QTimer(this);
    // connect(timer, SIGNAL(timeout()), this, SLOT(update()));
    // this->timer->start(100);
    // iTimerId = startTimer(200);
}
//------------------------------------------------------------------------------
void TClock::IRQ_update(long lDelta_ms)
{
    ulTimeCntr += static_cast<unsigned long> (lDelta_ms);
}
//------------------------------------------------------------------------------
//void TClock::timerEvent(QTimerEvent *event)
//{

// if (event->timerId() == iTimerId)
// {
//    update();
// }
//}
// max signed long  2147483647
// -----------------------------------------------------------------------------
/**
 * @brief Constructor - initialization as default
 * @pre 	tClock has to be already declared and implemented
 */
TChronoMeter::TChronoMeter()
    : 	universalClock(&tClock),
      u32Tick0(0),
      u32Tick1(0),
      bPaused(0),
      fTicPrescaler(1)
{
    start();
}
// -----------------------------------------------------------------------------
/**
 * @brief 	Destructor - deInitialization
 */
TChronoMeter::~TChronoMeter()
{
    stopReset();
}
// -----------------------------------------------------------------------------
/**
 * @brief Resume and continues to count clocks
 */
void TChronoMeter::start(void)
{
    bPaused = false;
    u32Tick0 = universalClock->watch();
}
// -----------------------------------------------------------------------------
/**
 * @brief Stops counting clocks WITHOUT resetting counters
 */
void TChronoMeter::pause(void)
{
    if (bPaused == false)
        u32Tick1 = universalClock->watch();
    bPaused = true;
}
// -----------------------------------------------------------------------------
/**
 * @brief Resumes counting clocks WITHOUT resetting counters
 */
void TChronoMeter::resume(void)
{
    if (bPaused == true)
        u32Tick0 = universalClock->watch() - (u32Tick1 - u32Tick0);
    bPaused = false;
}
// -----------------------------------------------------------------------------
/**
 * @brief Stops counting clocks WITHOUT resetting counters
 */
float TChronoMeter::watch(void)
{
    float fDelta;
    if (bPaused == false)
        u32Tick1 = universalClock->watch();
    //	if (u32Tick1 < u32Tick0)
    //		delta = 0;
    //	else
    fDelta = u32Tick1 - u32Tick0;

    return(fDelta / fTicPrescaler);
}
// -----------------------------------------------------------------------------
/**
 * @brief Stops counting clocks WITHOUT resetting counters
 */
bool TChronoMeter::exceed(float fTimeThreshold)
{
    bool bVar = 0;
    float fDelta = watch(); // ms
    if ((!bPaused) && (fDelta >= fTimeThreshold))
        bVar = 1;
    return(bVar);
}
// -----------------------------------------------------------------------------
/**
 * @brief Stops counting clocks and resetting counters
 */
void TChronoMeter::stopReset(void)
{
    u32Tick0 = universalClock->watch();
    u32Tick1 = u32Tick0;
    bPaused = false;
}
// -----------------------------------------------------------------------------
/**
 * @brief Estimate the actual time now TIME
 */
float TChronoMeter::now(void)
{
    //	DBG		static long long emulTime = 0;
    //	DBG		return(emulTime++);
    return(universalClock->watch()/fTicPrescaler);
}
