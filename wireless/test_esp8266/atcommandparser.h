#ifndef ATCOMMANDPARSER_H
#define ATCOMMANDPARSER_H

#include <QObject>

#include "actions.h"

#define RX_BUFFER_SIZE (150u)

typedef char DataItem;

enum ParserState {
    StateUnknown = 0,
    StateWaitReady,
    StateWaitAT,
    StateWaitRawData,
    StateSendRawData,
    StateWaitString
};

class AtCommandParser : public QObject
{
    Q_OBJECT
public:
    explicit AtCommandParser(QObject *parent = nullptr);
    void putData(DataItem dataItem);
    void analyseData();

private:
    void parseData();
    void dropCRLF();
    bool isStrEqualToBuf(const char *str, unsigned from=0);
    bool isStrInBufComplete();

    void setParserState(ParserState newState);
    void gotReady();
    void gotIpd();
    void gotPrompt();
    void gotFullCommand();

    void updateTable(int *out_updatedEventsCount=nullptr);
    void clearTable(int *out_clearedEventsCount=nullptr);
    int getActionId(ActionId id);
    unsigned getActionHit(ActionId id);
    bool takeActionHit(ActionId id);
    void clearActionHit(ActionId id);

    DataItem m_rxBuffer[RX_BUFFER_SIZE];
    uint32_t m_rxIndex;

    bool m_isReady;
    bool m_isOk;

    ParserState m_parserState;
};

#endif // ATCOMMANDPARSER_H
