#ifndef __SETTINGS_H__
#define __SETTINGS_H__

struct settings_t {
  int sock;
  char targetIp[80];
  char fastdIp[80];
  int daemonize;
  int check_mtu;
  int set_mtu;
  int response_timeout;
  int interval;
  int simulate;
};

#endif //__SETTINGS_H__

