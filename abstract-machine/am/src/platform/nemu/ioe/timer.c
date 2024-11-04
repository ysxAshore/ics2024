#include <am.h>
#include <nemu.h>
#include <time.h>
void __am_timer_init()
{
  outl(RTC_ADDR, 0);
  outl(RTC_ADDR + 4, 0);
}

void __am_timer_uptime(AM_TIMER_UPTIME_T *uptime)
{

  uint32_t high32 = inl(RTC_ADDR + 4);
  uint32_t low32 = inl(RTC_ADDR);
  uptime->us = ((uint64_t)high32 << 32) | low32;
}

void __am_timer_rtc(AM_TIMER_RTC_T *rtc)
{
  rtc->second = 0;
  rtc->minute = 0;
  rtc->hour = 0;
  rtc->day = 0;
  rtc->month = 0;
  rtc->year = 1900;
}
