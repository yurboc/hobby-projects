#include "atcommandparser.h"

#include <string>

extern ParserAction actionTable[];

AtCommandParser::AtCommandParser(QObject *parent) : QObject(parent)
{
    reset();
}

void AtCommandParser::reset()
{
    // Clear commands buffer
    dropBuffer();

    // Set initial parser state
    setParserState(StateWaitReady);

    // Reset actions table
    clearTable();

    // Clear pending command
    m_pendingAtCommand.clear();
}

void AtCommandParser::putData(DataItem dataItem)
{
    if (m_rxIndex >= RX_BUFFER_SIZE)
        m_rxIndex = 0;

    // Store RX data
    m_rxBuffer[m_rxIndex++] = dataItem;

    // Parse data
    parseData();
}

void AtCommandParser::setPendingCommand(QString commandName)
{
    m_pendingAtCommand = commandName;
}

void AtCommandParser::parseData()
{
    // Wait for "ready" string
    if (m_parserState == StateWaitReady) {

        // Drop CR and LF at begin of line
        dropCRLF();

        // Skip not finished lines
        if (!isStrInBufComplete())
            return;

        // Find "ready"
        if (!isStrEqualToBuf(str_ready)) {
            dropBuffer();
            return;
        }

        // Got "ready"
        gotReady();
        dropBuffer();
        return;
    }

    // Wait for known AT commands
    if (m_parserState == StateWaitAT) {

        // Drop CR and LF at begin of line
        dropCRLF();

        // Find "+IPD,"
        if (isStrEqualToBuf(str_ipd)) {
            gotIpd();
            return;
        }

        // Find ">"
        if (isStrEqualToBuf(str_prompt)) {
            gotPrompt();
            return;
        }

        // Skip not finished lines
        if (!isStrInBufComplete())
            return;

        // Parse full string
        gotFullCommand();

        // Clear buffer
        dropBuffer();
        return;
    }
}

void AtCommandParser::setParserState(ParserState newState)
{
    m_parserState = newState;
}

void AtCommandParser::dropCRLF()
{
    if (m_rxIndex == 1
            && (m_rxBuffer[0] == '\r' || m_rxBuffer[0] == '\n')) {
        m_rxIndex = 0;
        return;
    }
}

void AtCommandParser::dropBuffer()
{
    m_rxIndex = 0;
    memset(m_rxBuffer, 0, sizeof(DataItem) * sizeof(RX_BUFFER_SIZE));
}

bool AtCommandParser::isStrEqualToBuf(const char *str, unsigned from)
{
    const char *bufferPtr = static_cast<const char*>(m_rxBuffer);

    if (from >= RX_BUFFER_SIZE || str == nullptr || m_rxIndex <= 2)
        return false;

    bufferPtr += from;

    int res = strncmp(bufferPtr, str, RX_BUFFER_SIZE - from);

    if (res == 0)
        return true;

    return false;
}

bool AtCommandParser::isStrInBufComplete()
{
    if (m_rxIndex >= 2
            && m_rxBuffer[m_rxIndex-2] == '\r'
            && m_rxBuffer[m_rxIndex-1] == '\n') {

        // Replace CR LF by terminator
        m_rxBuffer[m_rxIndex-2] = '\0';
        m_rxBuffer[m_rxIndex-1] = '\0';
        return true;
    }
    return false;
}

void AtCommandParser::gotReady()
{
    setParserState(StateWaitAT);
    emit gotNewState("READY");
}

void AtCommandParser::gotIpd()
{
    // [TODO] parse channel and data length
    setParserState(StateWaitRawData);
}

void AtCommandParser::gotPrompt()
{
    setParserState(StateSendRawData);
}

void AtCommandParser::gotFullCommand()
{
    // Parse all known states and AT commands
    int hits = 0;
    updateTable(&hits);

    // Handle "pending command"
    bool gotOk = takeActionHit(ACTION_OK);
    bool gotError = takeActionHit(ACTION_ERROR);
    if (!m_pendingAtCommand.isEmpty()) {
        if (gotError) {
            emit gotNewState("!"+m_pendingAtCommand);
        }
        else if (gotOk) {
            emit gotNewState(m_pendingAtCommand);
        }
    }
}

void AtCommandParser::updateTable(int *out_updatedEventsCount)
{
    int totalHitCount = 0;
    for (int i = 0; i < ACTION_MAX; ++i) {
        if (isStrEqualToBuf(actionTable[i].cmdStr, actionTable[i].from)) {
            ++totalHitCount;
            ++actionTable[i].hit;
        }
    }
    if (out_updatedEventsCount != nullptr) {
        *out_updatedEventsCount = totalHitCount;
    }
}

void AtCommandParser::clearTable(int *out_clearedEventsCount)
{
    int totalHitCount = 0;
    for (int i = 0; i < ACTION_MAX; ++i) {
        totalHitCount += actionTable[i].hit;
        actionTable[i].hit = 0;
    }
    if (out_clearedEventsCount != nullptr) {
        *out_clearedEventsCount = totalHitCount;
    }
}

int AtCommandParser::getActionId(ActionId id)
{
    for (int i = 0; i < ACTION_MAX; ++i) {
        if (actionTable[i].actionId == id)
            return i;
    }
    return -1; // wrong ActionId
}

unsigned AtCommandParser::getActionHit(ActionId id)
{
    int actualId = getActionId(id);
    if (actualId < 0)
        return 0; // wrong ActionId

    return actionTable[actualId].hit;
}

bool AtCommandParser::takeActionHit(ActionId id)
{
    int actualId = getActionId(id);
    if (actualId < 0)
        return false; // wrong ActionId

    if (actionTable[actualId].hit == 0)
        return false;

    --actionTable[actualId].hit;
    return true;
}

void AtCommandParser::clearActionHit(ActionId id)
{
    int actualId = getActionId(id);
    if (actualId < 0)
        return; // wrong ActionId

    actionTable[actualId].hit = 0;
}
