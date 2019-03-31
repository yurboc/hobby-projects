#ifndef ACTIONS_H
#define ACTIONS_H

enum ActionId {
    ACTION_READY,
    ACTION_OK,
    ACTION_ERROR,
    ACTION_BUSY_P,
    ACTION_BUSY_S,
    ACTION_FAIL,
    ACTION_SEND_OK,
    ACTION_SEND_FAIL,
    ACTION_WIFI_DISCONNECT,
    ACTION_WIFI_CONNECT,
    ACTION_WIFI_GOTIP,
    ACTION_CIFSR_STAIP,
    ACTION_CIFSR_APIP,
    ACTION_CIPSTAMAC,
    ACTION_CIPAPMAC,
    ACTION_CONNECT,
    ACTION_CONNECT_FAIL,
    ACTION_CLOSED,
    ACTION_CWLAP,
    ACTION_CWJAP,
    ACTION_IPD,
    ACTION_AT_RST,
    ACTION_AT_CIFSR,
    ACTION_AT_CWLAP,
    ACTION_AT_VERSION,
    ACTION_AT_CIPSTAMAC,
    ACTION_PROMPT,

    ACTION_MAX
};

struct ParserAction {
    ActionId actionId;
    const char* cmdStr;
    unsigned from;
    unsigned hit;
};

const char str_prompt[] = ">";
const char str_ready[] = "ready";
const char str_ok[] = "OK";
const char str_error[] = "ERROR";
const char str_busy_p[] = "busy p...";
const char str_busy_s[] = "busy s...";
const char str_fail[] = "FAIL";
const char str_send_ok[] = "SEND OK";
const char str_send_fail[] = "SEND FAIL";
const char str_staip[] = "+CIFSR:STAIP,";
const char str_apip[] = "+CIFSR:APIP,";
const char str_stamac[] = "+CIPSTAMAC:";
const char str_apmac[] = "+CIPAPMAC:";
const char str_cwlap[] = "+CWLAP:";
const char str_at_rst[] = "AT+RST";
const char str_at_cifsr[] = "AT+CIFSR";
const char str_connect[] = ",CONNECT";
const char str_connect_fail[] = ",CONNECT FAIL";
const char str_closed[] = ",CLOSED";
const char str_wifi_disconn[] = "WIFI DISCONNECT";
const char str_wifi_connect[] = "WIFI CONNECT";
const char str_wifi_got_ip[] = "WIFI GOT IP";
const char str_at_cwlap[] = "AT+CWLAP";
const char str_at_version[] = "AT+VERSION";
const char str_at_cipstamac[] = "AT+CIPSTAMAC?";
const char str_cwjap[] = "+CWJAP:";
const char str_c_send_ok[] = ",SEND OK";
const char str_ipd[] = "+IPD,";

#endif // ACTIONS_H
