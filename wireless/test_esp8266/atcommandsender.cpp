#include "atcommandsender.h"

extern ParserAction actionTable[];

AtCommandSender::AtCommandSender()
{
    reset();
}

void AtCommandSender::reset()
{
    m_currentState = "unknown";
}

void AtCommandSender::addState(QString name)
{
    m_states.push_back(name);
}

void AtCommandSender::addCommand(QString name, QString cmd)
{
    m_commands.insert(std::pair<QString,QString>(name, cmd));
}

void AtCommandSender::addTransition(ActionId onAction, QString from, QString to)
{
    m_transitions.push_back(Transition(onAction, from, to));
}

bool AtCommandSender::updateByRequest(QString request)
{
    if (request == "RST") {
        m_currentState = "RST";
        return true;
    }

    return false;
}

bool AtCommandSender::updateByState()
{
    for (auto it = m_transitions.begin(); it != m_transitions.end(); it++) {
        Transition currentTransition = *it;

        // Check source state
        if (currentTransition.oldState != m_currentState) {
            continue;
        }

        // Read actions table
        for (int i = 0; i < ACTION_MAX; i++) {

            // Skip actions with another ID
            if (actionTable[i].actionId != currentTransition.action) {
                continue;
            }

            // Skip actions that not hit
            if (actionTable[i].hit == 0) {
                continue;
            }

            // Update state
            m_currentState = currentTransition.newState;
            return true;
        }
    }

    // No transitions performed
    return false;
}
