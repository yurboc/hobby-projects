#ifndef ATCOMMANDSENDER_H
#define ATCOMMANDSENDER_H

#include <QObject>

#include "actions.h"

struct Transition {
    Transition(ActionId act, QString os, QString ns) {
        action = act;
        oldState = os;
        newState = ns;
    }

    ActionId action;
    QString oldState;
    QString newState;
};

class AtCommandSender : public QObject
{
    Q_OBJECT
public:
    AtCommandSender();
    void reset();

    void addState(QString name);
    void addCommand(QString name, QString cmd);
    void addTransition(ActionId onAction, QString from, QString to);

    bool updateByRequest(QString request);
    bool updateByState();
    QString getNextCommand();

private:
    std::map<QString, QString> m_commands;
    std::list<Transition> m_transitions;
    std::list<QString> m_states;

    QString m_currentState;
};

#endif // ATCOMMANDSENDER_H
