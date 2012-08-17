#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include "cpugov.h"
#include "main.h"

unsigned int lowfreq, highfreq;
unsigned long long int lowtime, hightime;
static char lowfreqstr[STRBUFF_SIZE], highfreqstr[STRBUFF_SIZE];
static unsigned int lowfreqstr_size, highfreqstr_size;
static int corefds[MAX_CORES];
static int cgroupfreezefd;
unsigned int finished = 0;

#define DO_NOTHING() \
while(!finished) \
  sleep(2);

/* Using of cores and corenb global var (see cpugov.h) */
int init_cores()
{
  unsigned int tmp;
  char filenamebuff[STRBUFF_SIZE];

  tmp = corenb;

  while (tmp--)
  {
    snprintf(filenamebuff,sizeof(filenamebuff),
      "/sys/devices/system/cpu/cpu%d/cpufreq/scaling_setspeed",cores[tmp]);
    corefds[tmp] = open(filenamebuff,O_WRONLY);
  }
  reset_frequency();

  return 0;
}

__inline__ int set_frequency_low()
{
  static unsigned int corei;

  corei = corenb;

  while (corei--)
    write(corefds[corei],lowfreqstr,lowfreqstr_size);

  return 0;
}

__inline__ int set_frequency_high()
{
  static unsigned int corei;

  corei = corenb;

  while (corei--)
    write(corefds[corei],highfreqstr,highfreqstr_size);

  return 0;
}

__inline__ int set_frequency(unsigned int frequency)
{
  unsigned int corei, strbuff_size;
  char strbuff[STRBUFF_SIZE];
  
  strbuff_size = snprintf(strbuff,sizeof(strbuff),"%d",frequency);

  corei = corenb;

  while (corei--)
    write(corefds[corei],strbuff,strbuff_size);

  return 0;
}

__inline__ int reset_frequency()
{
  char filenamebuff[STRBUFF_SIZE];
  unsigned int tmp;

  tmp = corenb;

  while (tmp--)
  {
    snprintf(filenamebuff,sizeof(filenamebuff),
      "cpufreq-set -c %d -f %d",cores[tmp],freq_max);
    system(filenamebuff);
  }
}

__inline__ int cgroup_freeze()
{
  if (write(cgroupfreezefd,CGROUP_FREEZE,sizeof(CGROUP_FREEZE)) <= 0)
  {
        perror(__func__);
        exit(1);
  }

  return 0;
}

__inline__ int cgroup_thaw()
{
  if (write(cgroupfreezefd,CGROUP_THAW,sizeof(CGROUP_THAW)) <= 0)
  {
        perror(__func__);
        exit(1);
  }

  return 0;
}

int cycle()
{
  if (lowfreq == 0)
    cgroup_freeze();
  else
    set_frequency_low();

  usleep(lowtime);

  if (lowfreq == 0)
    cgroup_thaw();
  else
    set_frequency_high();

  usleep(hightime);

  return 0;
}

void stop(int num)
{
  int tmp;

  tmp = corenb;
  while (tmp--)
    close(corefds[tmp]);
  close(cgroupfreezefd);
  reset_frequency();
  finished = 1;
}

int extrun(unsigned long long pitch, unsigned int freqlow, unsigned int freqhigh, double ratelow, char *cgroup_path)
{
  char strbuff[STRBUFF_SIZE];

  if (signal(SIGINT, stop) == SIG_ERR || signal(SIGTERM, stop) == SIG_ERR)
    return 1;
  init_cores();
  lowfreq = freqlow;
  lowfreqstr_size = snprintf(lowfreqstr,sizeof(lowfreqstr),"%d",lowfreq);
  highfreq = freqhigh;
  highfreqstr_size = snprintf(highfreqstr,sizeof(highfreqstr),"%d",highfreq);
  
  if (ratelow == 0.0f)
  {
    set_frequency_high();
    DO_NOTHING();
  }
  else if (ratelow == 1.0f)
  {
    set_frequency_low();
    DO_NOTHING();
  }
  else
  {
    lowtime = (__typeof__(lowtime)) (pitch * ratelow);
    hightime = (__typeof__(hightime)) (pitch * (1-ratelow));

    snprintf(strbuff,sizeof(strbuff),"%s/freezer.state",cgroup_path);
    cgroupfreezefd = open(strbuff,O_WRONLY);

    printf("pitch: %dus, freqlow: %d kHz, freqhigh: %d KHz, timelow: %dus, timehigh: %dus\n",pitch,lowfreq,highfreq,lowtime,hightime);

    set_frequency_high();
    while (!finished)
      cycle();
  }
  return 0;
}
