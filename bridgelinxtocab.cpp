#include "bridgelinxtocab.h"

BridgeLinxtoCab::BridgeLinxtoCab(QObject *parent) : QObject(parent)
{
    QSettings settings("MakroSoft", "MakroRetranslator");
    Constants::labelTemplate = settings.value("label/template").toString();
}

// Основной метод обработки команд Linx
void BridgeLinxtoCab::processLinxCommand(const QByteArray &raw)
{
    if (raw.isEmpty()) {
        emit logMessage("Пустые данные пришли");
        emit responseToMakroline("ERR|EMPTY_DATA");
        return;
    }

    QByteArray fixedRaw = raw;
    if (fixedRaw.size() % 2 != 0) {
        fixedRaw.append('\0');
    }

    const QString rawUtf16 = QString::fromUtf16(
                                 reinterpret_cast<const char16_t*>(fixedRaw.constData()),
                                 fixedRaw.size() / 2
                                 ).trimmed();

    if (rawUtf16.isEmpty()) {
        emit logMessage("Ошибка конвертирования в UTF-16");
        emit responseToMakroline("ERR|INVALID_UTF16");
        return;
    }

    const QStringList individualCommands = rawUtf16.split('\r', Qt::SkipEmptyParts);
    if (individualCommands.isEmpty()) {
        emit logMessage("Не удалось выделить команды из пакета: " + rawUtf16);
        emit responseToMakroline("ERR|NO_VALID_COMMANDS");
        return;
    }

    emit logMessage("[System] В пакете найдено команд: " + QString::number(individualCommands.size()));

    for (const QString &singleCommand : individualCommands) {
        QString trimmedCommand = singleCommand.trimmed();
        if (trimmedCommand.isEmpty()) continue;

        emit logMessage("[System] Обработка команды: " + trimmedCommand);

        const QStringList parts = trimmedCommand.split('|');
        const QString command = parts.value(0).toUpper();

        static const QHash<QString, Constants::TypeLinxCommand> reverseCommandMap = [](){
            QHash<QString, Constants::TypeLinxCommand> map;
            for (auto it = Constants::LinxCommandText.begin(); it != Constants::LinxCommandText.end(); ++it) {
                map.insert(QLatin1String(it.value()), it.key());
            }
            return map;
        }();

        Constants::TypeLinxCommand cmdType = reverseCommandMap.value(command, Constants::TypeLinxCommand::Unknown);

        switch(cmdType) {
        case Constants::TypeLinxCommand::Print:
            emit logMessage("[System] Получена команда на печать");
            handlePrint();
            break;
        case Constants::TypeLinxCommand::RequestState:
            handleRequestState();
            break;
        case Constants::TypeLinxCommand::SelectJob:
            handleSelectJob(parts);
            break;
        case Constants::TypeLinxCommand::UpdateJobNamed:
            handleUpdateJobNamed(parts);
            break;
        case Constants::TypeLinxCommand::RequestAsyncState:
            handleRequestAsyncState();
            break;
        case Constants::TypeLinxCommand::RequestQueueSize:
            handleRequestQueueSize();
            break;
        case Constants::TypeLinxCommand::SetState:
            handleSetState(parts);
            break;
        case Constants::TypeLinxCommand::AddToBuffer:
        case Constants::TypeLinxCommand::AddToQueue:
            handleAddToBuffer(parts);
            break;
        case Constants::TypeLinxCommand::ClearFaults:
            handleClearFaults();
            break;
        case Constants::TypeLinxCommand::ClearQueue:
            handleClearQueue();
            break;
        case Constants::TypeLinxCommand::Unknown:
            handleUnknownCommand();
            break;
        default:
            break;
        }
    }
}

// ========================================================
// Трансформация Linx → CAB
// ========================================================
QByteArray BridgeLinxtoCab::transformLinxToCab(const QString &linxCommand)
{
    if (linxCommand.isEmpty()) return QByteArray();

    QStringList parts = linxCommand.split('|', Qt::SkipEmptyParts);
    QHash<QString, QString> values;
    QString rawCode;

    for (const QString &part : parts) {
        if (part.startsWith("code=")) {
            rawCode = part.mid(5).split('~').first();
        } else if (part.contains('=')) {
            QString key = part.section('=', 0, 0).trimmed();
            QString value = part.section('=', 1).trimmed();
            values[key] = value;
        }
    }

    QStringList cabLines;
    if (!rawCode.isEmpty()) cabLines << QString("R code;[U:FNC1]%1").arg(rawCode);
    if (values.contains("name")) cabLines << QString("R name;%1").arg(values["name"]);
    if (values.contains("batch")) cabLines << QString("R batch;%1").arg(values["batch"]);
    if (values.contains("production_datetime")) cabLines << QString("R production_datetime;%1").arg(values["production_datetime"]);
    if (values.contains("expiration_datetime")) cabLines << QString("R expiration_datetime;%1").arg(values["expiration_datetime"]);
    cabLines << "R weight;0"; // Заглушка, можно брать вес из переменной
    cabLines << "A 1";

    QString finalCommand = cabLines.join('\n');
    emit logMessage("[System] Сформирована команда CAB:\n" + finalCommand);

    return finalCommand.toUtf8();
}

void BridgeLinxtoCab::setWeightFromPLC(const QByteArray &data)
{
    if (data.isEmpty() || pendingCabQueue.isEmpty()) return;

    QString weightStr = QString::fromUtf8(data).trimmed();
    lastWeight = weightStr;
    Constants::lastWeight = weightStr;

    emit logMessage("[System] Получен вес от PLC: " + weightStr);

    // Отправляем все команды из очереди, вставляя вес
    // Вставляем вес в CAB-команду перед отправкой
    QString cmdStr = QString::fromUtf8(pendingCabCommand);
    cmdStr.replace("R weight;0", "R weight;" + weightStr); // заменяем заглушку

    QByteArray commandToSend = cmdStr.toUtf8();
    qDebug() << "CommandtoSend" <<commandToSend;
    // Отправляем команду в принтер
    emit commandToPrinter(commandToSend, Constants::TypeCommandCab::AddCode);
    emit logMessage("[System] Отправлена команда на печать с весом: " + weightStr);

    // Очищаем отложенную команду
    pendingCabCommand.clear();
}

// ========================================================
// Обработчики команд Linx
// ========================================================

void BridgeLinxtoCab::handleAddToBuffer(const QStringList &parts)
{
    if (parts.size() < 2) return;

    QString codeCommand = parts[1];
    QString rawCommand;

    if (codeCommand.startsWith("code=")) rawCommand = codeCommand.mid(5);
    else if (codeCommand.startsWith("SHD|code=")) rawCommand = codeCommand.mid(9);
    else return;

    makrolineQueue.enqueue(rawCommand);
    emit logMessage("[System] Добавлено кодов: 1, всего в очереди: " + QString::number(makrolineQueue.size()));

    // Генерируем команду CAB и кладем в очередь отложенных команд
    QByteArray cabCommand = transformLinxToCab(rawCommand);
    pendingCabQueue.enqueue(cabCommand);

    emit logMessage("[System] Команда CAB добавлена в очередь и ждёт вес");
    emit responseToMakroline("ACK");
}

void BridgeLinxtoCab::handlePrinterState(Constants::CabState)
{
    emit responseToMakroline(QString("STS|%1|0|gs1dm|%2|%3")
                                 .arg(static_cast<int>(3))
                                 .arg(Constants::countPrinted)
                                 .arg(Constants::countPrinted).toUtf8());
}

void BridgeLinxtoCab::handleAddToQueue(const QStringList &parts)
{
    handleAddToBuffer(parts);
}

void BridgeLinxtoCab::handleClearQueue()
{
    makrolineQueue.clear();
    emit responseToMakroline("ACK");
}

void BridgeLinxtoCab::handlePrint() { manualPrint(); }

void BridgeLinxtoCab::handleSelectJob(const QStringList &parts)
{
    QString jobName = parts.value(1).trimmed();
    Constants::variables.clear();

    for (int i = 2; i < parts.size(); ++i) {
        QString pair = parts[i];
        QString field = pair.section('=', 0, 0).trimmed();
        QString value = pair.section('=', 1).trimmed();
        if (!field.isEmpty()) Constants::variables[field] = value;
    }

    emit logMessage("[System] Задание выбрано: " + jobName);
    emit responseToMakroline("ACK");
}

void BridgeLinxtoCab::handleRequestState()
{
    QByteArray response = QString("STS|%1|0|DemoJob|%2|%3|")
    .arg(static_cast<int>(3))
        .arg(Constants::batchCount)
        .arg(Constants::totalCount)
        .toUtf8();
    emit responseToMakroline("STS|3|0|DemoJob|0|0|");
}

void BridgeLinxtoCab::handleRequestQueueSize()
{
    emit responseToMakroline(QString("SRC|%1").arg(makrolineQueue.size()).toUtf8());
}

void BridgeLinxtoCab::handleRequestAsyncState() { emit responseToMakroline("ACK"); }

void BridgeLinxtoCab::handleClearFaults()
{
    QByteArray cmd = getConvertStringToByte(Constants::TypeCommandCabText.value(Constants::TypeCommandCab::ClearBuffers));
    emit commandToPrinter(cmd, Constants::TypeCommandCab::ClearBuffers);
    emit responseToMakroline("ACK");
}

void BridgeLinxtoCab::handleSetState(const QStringList &parts)
{
    if (parts.size() < 2) { emit responseToMakroline("[System] ERS|Неверный формат команды SST"); return; }

    bool ok;
    int targetState = parts[1].toInt(&ok);
    if (!ok || targetState < 0 || targetState > 4) {
        emit responseToMakroline("[System] ERS|Недопустимое значение состояния");
        return;
    }

    Constants::LinxState requestedState = static_cast<Constants::LinxState>(targetState);
    if (!isValidStateTransition(Constants::currentStatusMakroline, requestedState)) {
        emit responseToMakroline("[System] ERS|Недопустимый переход состояний");
        return;
    }

    QByteArray docodCommand;
    switch(requestedState) {
    case Constants::LinxState::Running:
        docodCommand = Constants::TypeCommandCabText.value(Constants::TypeCommandCab::StartPrint);
        Constants::currentStatusCab = Constants::CabState::Start;
        emit commandToPrinter(docodCommand, Constants::TypeCommandCab::StartPrint);
        break;
    case Constants::LinxState::Shutdown:
    case Constants::LinxState::ShuttingDown:
    case Constants::LinxState::Offline:
        docodCommand = Constants::TypeCommandCabText.value(Constants::TypeCommandCab::StopPrint);
        Constants::currentStatusCab = Constants::CabState::Stop;
        emit commandToPrinter(docodCommand, Constants::TypeCommandCab::StopPrint);
        break;
    default: break;
    }

    Constants::currentStatusMakroline = requestedState;
    emit responseToMakroline("ACK");
}

void BridgeLinxtoCab::handleUnknownCommand() { emit responseToMakroline("ERR"); }

void BridgeLinxtoCab::handleUpdateJobNamed(const QStringList &parts) { emit responseToMakroline("SFS|951744|"); }

// ========================================================
// Вспомогательные методы
// ========================================================

QByteArray BridgeLinxtoCab::getConvertStringToByte(const QString& string)
{
    QByteArray result;
    const QStringList byteStrings = string.split(' ', Qt::SkipEmptyParts);
    for (const QString& byteStr : byteStrings) {
        bool ok;
        int byteValue = byteStr.toInt(&ok, 16);
        if (ok && byteValue >= 0 && byteValue <= 0xFF) result.append(static_cast<char>(byteValue));
        else emit logMessage("[System] Invalid hex byte: " + byteStr);
    }
    return result;
}

QString BridgeLinxtoCab::getErrorState() const { return QString(); }
QString BridgeLinxtoCab::getCurrentJob() const { return QString(); }

void BridgeLinxtoCab::manualPrint() {}
void BridgeLinxtoCab::autoPrinted() { QTimer::singleShot(0, this, &BridgeLinxtoCab::processPrintQueue); }
void BridgeLinxtoCab::processPrintQueue() {}
void BridgeLinxtoCab::changeAutoAndManualModes(bool checked) { Constants::AutoAndManualModes = checked; }
bool BridgeLinxtoCab::isValidStateTransition(Constants::LinxState current, Constants::LinxState target) const
{
    if (current == target) return true;
    switch (current) {
    case Constants::LinxState::Shutdown:    return target == Constants::LinxState::StartingUp || target == Constants::LinxState::ShuttingDown;
    case Constants::LinxState::StartingUp:  return target == Constants::LinxState::Running || target == Constants::LinxState::Shutdown;
    case Constants::LinxState::Running:     return target == Constants::LinxState::Offline || target == Constants::LinxState::ShuttingDown;
    case Constants::LinxState::Offline:     return target == Constants::LinxState::Running || target == Constants::LinxState::ShuttingDown;
    case Constants::LinxState::ShuttingDown:return target == Constants::LinxState::Shutdown || target == Constants::LinxState::StartingUp;
    default: return false;
    }
}
