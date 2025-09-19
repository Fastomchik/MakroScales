#include "bridgelinxtocab.h"

BridgeLinxtoCab::BridgeLinxtoCab(QObject *parent) : QObject(parent)
{
    QSettings settings("MakroSoft", "MakroRetranslator");
    Constants::labelTemplate = settings.value("label/template").toString();
    lastValues["name"] = "";
    lastValues["batch"] = "";
    lastValues["production_datetime"] = "";
    lastValues["expiration_datetime"] = "";
    lastRawCode = "";
}

BridgeLinxtoCab::~BridgeLinxtoCab()
{

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
    QHash<QString, QString> currentValues;
    QString currentRawCode;

    // Проходим по всем частям команды
    for (const QString &part : parts) {
        if (part.startsWith("code=")) {
            currentRawCode = part.mid(5).trimmed();
            // Убираем кавычки
            currentRawCode.remove('"');
            // Заменяем ~d029 на [U:GS]
            currentRawCode.replace("~d029", "[U:GS]");
            // Заменяем все оставшиеся ~ на [U:FNC1]
            currentRawCode.replace("~1", "[U:FNC1]");

            // Сохраняем текущий код
            if (!currentRawCode.isEmpty()) {
                lastRawCode = currentRawCode;
            }
        } else if (part.contains('=')) {
            QString key = part.section('=', 0, 0).trimmed();
            QString value = part.section('=', 1).trimmed();
            currentValues[key] = value;

            // Сохраняем текущее значение, если оно не пустое
            if (!value.isEmpty()) {
                lastValues[key] = value;
            }
        }
    }

    QStringList cabLines;

    // Используем текущий код, если он есть, иначе последний сохраненный
    QString codeToUse = !currentRawCode.isEmpty() ? currentRawCode : lastRawCode;
    if (!codeToUse.isEmpty())
        cabLines << QString("R code;%1").arg(codeToUse);

    // Для каждого поля используем текущее значение, если оно есть, иначе последнее сохраненное
    if (currentValues.contains("name") || !lastValues["name"].isEmpty())
        cabLines << QString("R name;%1").arg(!currentValues["name"].isEmpty() ? currentValues["name"] : lastValues["name"]);

    if (currentValues.contains("batch") || !lastValues["batch"].isEmpty())
        cabLines << QString("R batch;%1").arg(!currentValues["batch"].isEmpty() ? currentValues["batch"] : lastValues["batch"]);

    if (currentValues.contains("production_datetime") || !lastValues["production_datetime"].isEmpty())
        cabLines << QString("R production_datetime;%1").arg(!currentValues["production_datetime"].isEmpty() ? currentValues["production_datetime"] : lastValues["production_datetime"]);

    if (currentValues.contains("expiration_datetime") || !lastValues["expiration_datetime"].isEmpty())
        cabLines << QString("R expiration_datetime;%1").arg(!currentValues["expiration_datetime"].isEmpty() ? currentValues["expiration_datetime"] : lastValues["expiration_datetime"]);

    // Заглушка — вес будет подставлен позже
    cabLines << "R weight;0";
    cabLines << "A1";

    QString finalCommand;
    for (const QString &line : cabLines) {
        finalCommand += line + "\r\n";
    }

    emit logMessage("[System] Сформирована CAB-команда:\n" + finalCommand);
    emit logMessage("[System] Использованные значения: "
                    "Код: " + codeToUse + ", "
                                  "Наименование: " + (!currentValues["name"].isEmpty() ? currentValues["name"] : lastValues["name"]) + ", "
                                                                                                        "Партия: " + (!currentValues["batch"].isEmpty() ? currentValues["batch"] : lastValues["batch"]) + ", "
                                                                                                           "Дата производства: " + (!currentValues["production_datetime"].isEmpty() ? currentValues["production_datetime"] : lastValues["production_datetime"]) + ", "
                                                                                                                                                     "Срок годности: " + (!currentValues["expiration_datetime"].isEmpty() ? currentValues["expiration_datetime"] : lastValues["expiration_datetime"]));

    return finalCommand.toUtf8();
}

void BridgeLinxtoCab::setWeightFromPLC(const QByteArray &data)
{
    if (!m_clientConnected) {
        emit logMessage("[System] Нет соединения с клиентом");
        return;
        }

        if (data.isEmpty() || pendingCabQueue.isEmpty()) return;

        QString weightStr = QString::fromUtf8(data).trimmed();

        emit logMessage("[System] Получен вес от PLC: " + weightStr);

        bool ok;
        float weightInt = weightStr.toFloat(&ok);

        QString cmdStr = QString::fromUtf8(pendingCabQueue.dequeue());
        cmdStr.replace("R weight;0\r\n", "R weight;" + weightStr + "\r\n");

        QByteArray commandToSend = cmdStr.toUtf8();
        qDebug() << "CommandtoSend" <<commandToSend;
        // Отправляем команду в принтер
        emit commandToPrinter(commandToSend, Constants::TypeCommandCab::AddCode);
        emit logMessage("[System] Отправлена команда на печать с весом: " + weightStr);
        emit updateDisplayTotalCountCounter(1);
        emit updateDisplayBufferCodesCount(static_cast<int>(pendingCabQueue.size()));
        emit updateDisplayWeightCounter(weightInt);
}


// ========================================================
// Обработчики команд Linx
// ========================================================

void BridgeLinxtoCab::handleAddToBuffer(const QStringList &parts)
{
    if (parts.isEmpty()) return;

    // Ищем любой part, который начинается с code=
    QString rawCommand;
    for (const QString &part : parts) {
        if (part.startsWith("code=")) {
            rawCommand = part.mid(5).trimmed();
            break;
        }
    }

    if (rawCommand.isEmpty()) {
        emit logMessage("[System] Не удалось определить код из команды: " + parts.join('|'));
        emit responseToMakroline("ERR|INVALID_CODE");
        return;
    }

    // Добавляем исходный код в очередь Makroline
    makrolineQueue.enqueue(rawCommand);
    emit logMessage("[System] Добавлено кодов: 1, всего в очереди: " + QString::number(makrolineQueue.size()));

    // Генерируем CAB-команду с учётом всех полей
    QByteArray cabCommand = transformLinxToCab(parts.join('|'));
    pendingCabQueue.enqueue(cabCommand);
    emit updateDisplayBufferCodesCount(static_cast<int>(pendingCabQueue.size()));
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
    QByteArray response = QString("STS|%1|%2|DemoJob|%3|%4|")
    //.arg(3)
    .arg(static_cast<int>(Constants::currentStatusMakroline))  // 0-4 (LinxState)
        .arg(0)
        .arg(Constants::batchCount)
        .arg(Constants::totalCount)
        .toUtf8();  // Преобразование в QByteArray
    qDebug() << response;
    emit responseToMakroline(response);
}

void BridgeLinxtoCab::handleRequestQueueSize()
{
    emit responseToMakroline(QString("SRC|%1").arg(makrolineQueue.size()).toUtf8());
    emit updateDisplayBufferCodesCount(static_cast<int>(makrolineQueue.size()));
}

void BridgeLinxtoCab::handleRequestAsyncState() { emit responseToMakroline("ACK"); }

void BridgeLinxtoCab::handleClearFaults()
{
    QByteArray cmd = getConvertStringToByte(Constants::TypeCommandCabText.value(Constants::TypeCommandCab::ClearBuffers));
    emit commandToPrinter(cmd, Constants::TypeCommandCab::ClearBuffers);
    pendingCabQueue.clear();
    makrolineQueue.clear();
    emit updateDisplayBufferCodesCount(static_cast<int>(pendingCabQueue.size()));
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


void BridgeLinxtoCab::updateClientStatus(bool connected)
{
    m_clientConnected = connected;
}

void BridgeLinxtoCab::updateServerStatus(bool connected)
{
    m_serverConnected = connected;
}
