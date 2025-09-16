#include "bridgelinxtocab.h"



BridgeLinxtoCab::BridgeLinxtoCab(QObject *parent) : QObject(parent)
{
    QSettings settings("MakroSoft", "MakroRetranslator");
    Constants::labelTemplate = settings.value("label/template").toString();

    /*QTimer* m_updateTimer = new QTimer(this);
    connect(m_updateTimer, &QTimer::timeout, this, &BridgeLinxtoCab::updateAllCount);
    m_updateTimer->start(3000);*/
}

// SHD|code=12345|date_time=12.05.2001|timestamp=xxxxxx|SHD|code=12345|date_time=12.05.2001|timestamp=xxxxxx|SHD|code=12345|date_time=12.05.2001|timestamp=xxxxxx|
void BridgeLinxtoCab::processLinxCommand(const QByteArray &raw)
{
    if (raw.isEmpty()) {
        emit logMessage("Пустые данные пришли");
        emit responseToMakroline("ERR|EMPTY_DATA");
        return;
    }

    QByteArray fixedRaw = raw;
    if (fixedRaw.size() % 2 != 0) {
        emit logMessage("Добавляем нулевой байт в конец");
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

    // Разбиваем весь пакет на отдельные команды по \r
    const QStringList individualCommands = rawUtf16.split('\r', Qt::SkipEmptyParts);

    if (individualCommands.isEmpty()) {
        emit logMessage("Не удалось выделить команды из пакета: " + rawUtf16);
        emit responseToMakroline("ERR|NO_VALID_COMMANDS");
        return;
    }

    emit logMessage("[System] В пакете найдено команд: " + QString::number(individualCommands.size()));

    // Обрабатываем КАЖДУЮ команду из пакета по отдельности
    for (const QString &singleCommand : individualCommands) {
        QString trimmedCommand = singleCommand.trimmed();
        if (trimmedCommand.isEmpty()) {
            emit logMessage("Пропуск пустой подстроки после split по \\r");
            continue;
        }

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

        // КЛЮЧЕВОЕ ИЗМЕНЕНИЕ: Объединяем обработку AddToBuffer и AddToQueue
        switch(cmdType) {
        case Constants::TypeLinxCommand::Print:
            emit logMessage("[System] Получена команда от ПО: Сигнал на печать " + trimmedCommand);
            handlePrint();
            break;
        case Constants::TypeLinxCommand::RequestState:
            emit logMessage("[System] Получена команда от ПО: Запрос состояния " + trimmedCommand);
            handleRequestState();
            break;
        case Constants::TypeLinxCommand::SelectJob:
            emit logMessage("[System] Получена команда от ПО: Выбор задания " + trimmedCommand);
            handleSelectJob(parts);
            break;
        case Constants::TypeLinxCommand::UpdateJobNamed:
            emit logMessage("[System] Получена команда от ПО: Обновление полей " + trimmedCommand);
            handleUpdateJobNamed(parts);
            break;
        case Constants::TypeLinxCommand::RequestAsyncState:
            emit logMessage("[System] Получена команда от ПО: Запрос обновления состояния " + trimmedCommand);
            handleRequestAsyncState();
            break;
        case Constants::TypeLinxCommand::RequestQueueSize:
            emit logMessage("[System] Получена команда от ПО: Запрос размера буфера " + trimmedCommand);
            handleRequestQueueSize();
            break;
        case Constants::TypeLinxCommand::SetState:
            emit logMessage("[System] Получена команда от ПО: Смена состояния " + trimmedCommand);
            handleSetState(parts);
            break;

            // КЛЮЧЕВОЕ ИЗМЕНЕНИЕ: Объединяем обработку этих двух команд
        case Constants::TypeLinxCommand::AddToBuffer:
        case Constants::TypeLinxCommand::AddToQueue:
            emit logMessage("[System] Получена команда от ПО: Пополнение очереди " + trimmedCommand);
            handleAddToBuffer(parts);
            break;

        case Constants::TypeLinxCommand::ClearFaults:
            emit logMessage("[System] Получена команда от ПО: Сброс аварий " + trimmedCommand);
            handleClearFaults();
            break;
        case Constants::TypeLinxCommand::ClearQueue:
            emit logMessage("[System] Получена команда от ПО: Сброс очереди SCB " + trimmedCommand);
            handleClearQueue();
            break;
        case Constants::TypeLinxCommand::Unknown:
            emit logMessage("[System] Неизвестная команда от ПО " + trimmedCommand);
            handleUnknownCommand();
            break;
        default:
            emit logMessage("[System] Команда не требует обработки: " + trimmedCommand);
            break;
        }
    }
}

//STS|<overallstate>|<errorstate>|<currentjob>|<batchcount>|<totalcount>|<CR>
void BridgeLinxtoCab::handleRequestState()
{
    QByteArray response = QString("STS|%1|%2|DemoJob|%3|%4|")
    //.arg(3)
    .arg(static_cast<int>(Constants::currentStatusMakroline))  // 0-4 (LinxState)
        .arg(0)
        .arg(Constants::batchCount)
        .arg(Constants::totalCount)
        .toUtf8();  // Преобразование в QByteArray

    emit responseToMakroline(response);
}

// Принимает обновленный статус от принтера Docod
void BridgeLinxtoCab::handlePrinterState(Constants::CabState state)
{
    Constants::CabState newState = static_cast<Constants::CabState>(state);

    if (Constants::currentStatusCab != newState) {
        Constants::currentStatusCab = newState;
        // Можно добавить логгирование
        emit logMessage("[System] Новое состояние от принтера " + QString::number(static_cast<int>(newState)));
    }

    // Автоматический переход из ShuttingDown в Shut down
    if (Constants::currentStatusMakroline == Constants::LinxState::ShuttingDown &&
        Constants::currentStatusCab == Constants::CabState::Stop) {

        emit logMessage("[System] Автоматический переход из ShuttingDown в Shut down");
        Constants::currentStatusMakroline = Constants::LinxState::Shutdown;

        // Отправляем команду подтверждения изменения состояния
        emit responseToMakroline("ACK");
    }

    // Обновляем ответ Makroline
    emit responseToMakroline(QString("STS|%1|0|gs1dm|%2|%3")
                                 .arg(static_cast<int>(newState))
                                 .arg(Constants::countPrinted)
                                 .arg(Constants::countPrinted).toUtf8());
}

// Запрос смены состояния от софта
void BridgeLinxtoCab::handleSetState(const QStringList &parts)
{
    // Проверка формата команды
    if (parts.size() < 2) {
        emit responseToMakroline("[System] ERS|Неверный формат команды SST");
        return;
    }

    // Парсинг целевого состояния
    bool ok;
    int targetState = parts[1].toInt(&ok);

    // Проверка валидности состояния
    if (!ok || targetState < 0 || targetState > 4) {
        emit responseToMakroline("[System] ERS|Недопустимое значение состояния");
        return;
    }

    Constants::LinxState requestedState = static_cast<Constants::LinxState>(targetState);

    // Проверка допустимости перехода состояний
    if (!isValidStateTransition(Constants::currentStatusMakroline, requestedState)) {
        emit responseToMakroline("[System] ERS|Недопустимый переход состояний");
        return;
    }

    // Дополнительная проверка для состояния "Running"
    if (requestedState == Constants::LinxState::Running && Constants::labelTemplate.isEmpty()) {
        emit responseToMakroline("[System] ERS|Не задан шаблон этикетки");
        return;
    }

    // Формируем команду для Docod
    QByteArray docodCommand;
    switch(requestedState) {
    case Constants::LinxState::StartingUp:// 1: Starting up;
        break;
    case Constants::LinxState::Running:   //  3: Running
        docodCommand = Constants::TypeCommandCabText.value(Constants::TypeCommandCab::StartPrint);
        Constants::currentStatusCab = Constants::CabState::Start;
        emit commandToPrinter(docodCommand, Constants::TypeCommandCab::StartPrint);
        break;

    case Constants::LinxState::Shutdown:     // 0: Shut down
    case Constants::LinxState::ShuttingDown: // 2: Shutting down
    case Constants::LinxState::Offline:      // 4: Off-line
        docodCommand = Constants::TypeCommandCabText.value(Constants::TypeCommandCab::StopPrint);
        Constants::currentStatusCab = Constants::CabState::Stop;
        emit commandToPrinter(docodCommand, Constants::TypeCommandCab::StopPrint);
        break;

    default:
        emit responseToMakroline("[System] ERS|Неизвестное состояние");
        return;
    }

    // Обновляем текущее состояние
    Constants::currentStatusMakroline = requestedState;

    // Отправляем подтверждение
    emit responseToMakroline("ACK");

    // Логирование изменения состояния
    emit logMessage("[System] Изменение состояние Makroline: " + QString::number(static_cast<int>(Constants::currentStatusMakroline)));
    emit logMessage("[System] Изменение состояние Docod " + QString::number(static_cast<int>(Constants::currentStatusCab)));
    emit logMessage("[System] Отправлена команда: " + QString(docodCommand));
}

void BridgeLinxtoCab::handleRequestQueueSize() {
    emit logMessage("[System] Количество кодов в буфере принтера: " + QString::number(Constants::countBufferInPrinter));
    emit responseToMakroline(QString("SRC|%1").arg(Constants::countBufferInPrinter).toUtf8());
}

void BridgeLinxtoCab::handlePrint()
{
    manualPrint();
}

void BridgeLinxtoCab::handleSelectJob(const QStringList &parts)
{
    QString jobName = parts.value(1).trimmed();

    Constants::variables.clear();
    for (int i = 2; i < parts.size(); ++i) {
        QString pair = parts[i];
        QString field = pair.section('=', 0, 0).trimmed();
        QString value = pair.section('=', 1).trimmed();
        if (!field.isEmpty()) {
            Constants::variables[field] = value;
        }
    }
    emit logMessage("[System] Задание выбрано: " + jobName);
    QString variablesStr;
    for (auto it = Constants::variables.constBegin(); it != Constants::variables.constEnd(); ++it) {
        variablesStr += it.key() + "=" + it.value() + "; ";
    }
    emit logMessage("[System] Переменные задания: " + variablesStr);
    emit responseToMakroline("ACK");
}

void BridgeLinxtoCab::handleRequestAsyncState()
{
    emit responseToMakroline("ACK");
}

void BridgeLinxtoCab::handleClearFaults()
{
    auto str = Constants::TypeCommandCabText.value(Constants::TypeCommandCab::ClearBuffers);
    QByteArray cmd = getConvertStringToByte(str);
    emit commandToPrinter(cmd, Constants::TypeCommandCab::ClearBuffers);
    emit responseToMakroline("ACK");
}

void BridgeLinxtoCab::handleAddToBuffer(const QStringList &parts)
{
    if (parts.size() < 2) {
        emit logMessage("[System] Неверное количество parts: " + QString::number(parts.size()));
        return;
    }

    // parts[1] теперь содержит строку с одним кодом, например:
    // "~10104911205084589215\"M&l1X*ye;Wn~d02993dGVz|"
    QString codeCommand = parts[1];
    QString rawCode;

    // Обрабатываем единственную команду
    if (codeCommand.startsWith("code=")) {
        rawCode = codeCommand.mid(5).remove("~d029").remove("\x1D");
    }
    else if (codeCommand.startsWith("SHD|code=")) {
        rawCode = codeCommand.mid(9).remove("~d029").remove("\x1D");
    }
    else {
        // Пропускаем некорректную команду
        emit logMessage("[System] Неизвестный формат команды: " + codeCommand);
        return;
    }

    if (rawCode.isEmpty()) {
        emit logMessage("[System] Код с пустыми параметрами");
        return;
    }

    // Добавляем код в очередь
    makrolineQueue.enqueue(rawCode);

    // Логируем состояние очереди
    emit logMessage("[System] Добавлено кодов: 1, всего в очереди: " + QString::number(makrolineQueue.size()));

    // Генерируем команду — она сама снимет первый элемент очереди
    QByteArray cmd = generateCommandFromTemplate();

    if (!cmd.isEmpty()) {
        emit commandToPrinter(cmd, Constants::TypeCommandCab::AddCode);
        emit responseToMakroline("ACK");
    } else {
        emit logMessage("[System] Сгенерированная команда пуста!");
    }
}


void BridgeLinxtoCab::handleAddToQueue(const QStringList &parts)
{
    // Аналогично handleAddToBuffer, если логика совпадает
    handleAddToBuffer(parts);
}


void BridgeLinxtoCab::handleClearQueue()
{
    QByteArray cmd = Constants::TypeCommandCabText.value(Constants::TypeCommandCab::ClearBuffers);
    emit logMessage(QString(cmd));
    emit commandToPrinter(cmd, Constants::TypeCommandCab::ClearBuffers);
    makrolineQueue.clear();
    emit responseToMakroline("ACK");
    //emit sendrequestBufferSizeTSC(&printedBufferSize);
}

void BridgeLinxtoCab::handleUnknownCommand()
{
    emit responseToMakroline("ERR");
}

void BridgeLinxtoCab::handleUpdateJobNamed(const QStringList &parts)
{
    emit responseToMakroline("SFS|951744|");
}

// ========================================================
// Вспомогательные методы
// ========================================================

QByteArray BridgeLinxtoCab::generateCommandFromTemplate()
{
    if (makrolineQueue.isEmpty()) {
        return QByteArray();
    }
    QString rawCode = makrolineQueue.dequeue(); // Берет и УДАЛЯЕТ первый элемент из очереди
    QString formattedCode;

    // 1. Форматируем код с сохранением структуры [XX]значение
    const QVector<Constants::CodeField> processingOrder = {
        Constants::CodeField::Field01,
        Constants::CodeField::Field21,
        Constants::CodeField::Field93
    };

    QString remainingCode = rawCode;
    for (Constants::CodeField field : processingOrder) {
        QString fieldCode = Constants::fieldMap.key(field);
        int fieldLength = Constants::fieldLengths.value(field);

        int pos = remainingCode.indexOf(fieldCode);
        if (pos >= 0 && pos + 2 + fieldLength <= remainingCode.length()) {
            formattedCode += QString("[%1]%2")
            .arg(fieldCode)
                .arg(remainingCode.mid(pos + 2, fieldLength));

            remainingCode.remove(pos, 2 + fieldLength);
        }
    }

    if (formattedCode.isEmpty()) {
        emit logMessage("[System] Не удалось отформатировать код: " + rawCode);
        return QByteArray();
    }

    // 2. Если шаблон пустой, используем отформатированный код как есть
    QString rendered = Constants::labelTemplate.isEmpty()
                           ? formattedCode
                           : Constants::labelTemplate;

    // 3. Заменяем плейсхолдеры в шаблоне (если есть)
    if (!Constants::labelTemplate.isEmpty()) {
        for (auto it = Constants::fieldMap.constBegin(); it != Constants::fieldMap.constEnd(); ++it) {
            const QString& fieldCode = it.key();
            QString fieldVar = QString("%Field%1%").arg(fieldCode);

            QRegularExpression re(QString("\\[%1\\]([^\\[]+)").arg(fieldCode));
            QRegularExpressionMatch match = re.match(formattedCode);

            if (match.hasMatch()) {
                rendered.replace(fieldVar, QString("[%1]%2").arg(fieldCode).arg(match.captured(1)));
            }
        } // Добавлена закрывающая скобка для цикла for
    } // Добавлена закрывающая скобка для if

    emit logMessage("[System] Итоговая команда: " + rendered);

    if (!rendered.isEmpty()) {
        Constants::idLabel++;
        emit logMessage("[System] Счётчик IDLABEL: " + QString::number(Constants::idLabel));
        return rendered.toUtf8();
    }

    return QByteArray();
}

bool BridgeLinxtoCab::isValidStateTransition(Constants::LinxState current, Constants::LinxState target) const
{
    if (current == target) {
        return true;
    }
    // Allowed state transitions according to the SST command specification
    switch (current) {
    case Constants::LinxState::Shutdown:    // 0: Shut down
        return target == Constants::LinxState::StartingUp ||
               target == Constants::LinxState::ShuttingDown;  // Only allowed to transition to 1: Starting up

    case Constants::LinxState::StartingUp:  // 1: Starting up
        return target == Constants::LinxState::Running ||    // Can transition to 3: Running
               target == Constants::LinxState::Shutdown;     // Or back to 0: Shut down

    case Constants::LinxState::Running:     // 3: Running
        return target == Constants::LinxState::Offline ||    // Can transition to 4: Off-line
               target == Constants::LinxState::ShuttingDown; // Or to 2: Shutting down

    case Constants::LinxState::Offline:     // 4: Off-line
        return target == Constants::LinxState::Running ||    // Can transition back to 3: Running
               target == Constants::LinxState::ShuttingDown;  // Or to 2: Shutting down

    case Constants::LinxState::ShuttingDown: // 2: Shutting down
        return target == Constants::LinxState::Shutdown ||
               target == Constants::LinxState::StartingUp;     // Only allowed to transition to 0: Shut down

    default:
        return false;  // Invalid current state (should never happen with proper enum usage)
    }
}


void BridgeLinxtoCab::manualPrint()
{
    /*if (!printQueue.isEmpty() && !Constants::AutoAndManualModes)
    {
        QByteArray current = printQueue.dequeue();
        emit logMessage("[Data] DOCOD Осталось в буфере PrintBuffer " + QString::number(printQueue.size()));
        //emit commandToPrinter(current);
        Constants::countPrinted++;
    }*/
}

void BridgeLinxtoCab::autoPrinted()
{
    QTimer::singleShot(0, this, &BridgeLinxtoCab::processPrintQueue);
}

void BridgeLinxtoCab::processPrintQueue()
{
}

void BridgeLinxtoCab::changeAutoAndManualModes(bool checked)
{
    Constants::AutoAndManualModes = checked;
    if (Constants::AutoAndManualModes && !printQueue.isEmpty()) {
        autoPrinted();
        emit logMessage("[System] Cмена ручного режима на Автоматический " + QString(checked ? "true" : "false"));
    }
}


void BridgeLinxtoCab::updateAllCount()
{
    emit logMessage("[System] Обновление счётчиков:\n"
                    "Буфер печати принтера (исскуственная): " + QString::number(printQueue.size()) + "\n"
                                                           "Буфер печати макролайн (исскуственная): " + QString::number(makrolineQueue.size()) + "\n"
                                                               "Бефера принтера: " + QString::number(Constants::countBufferInPrinter));
    emit updateSpinBox(Constants::SpinBoxType::MakrolineBuffer, makrolineQueue.size());
    emit updateSpinBox(Constants::SpinBoxType::PrinterBuffer, printQueue.size());
    emit updateSpinBox(Constants::SpinBoxType::ResponsePrinterBuffer, Constants::countBufferInPrinter);
}

//-----------------------------Геттеры-------------------------------------
QByteArray BridgeLinxtoCab::getConvertStringToByte(const QString& string)
{
    QByteArray result;
    const QStringList byteStrings = string.split(' ', Qt::SkipEmptyParts);

    for (const QString& byteStr : byteStrings) {
        bool ok;
        int byteValue = byteStr.toInt(&ok, 16);
        if (ok && byteValue >= 0 && byteValue <= 0xFF) {
            result.append(static_cast<char>(byteValue));
        } else {
            emit logMessage("[System] Invalid hex byte: " + byteStr);
        }
    }

    return result;
}


QString BridgeLinxtoCab::getErrorState() const
{
    return QString();
}

QString BridgeLinxtoCab::getCurrentJob() const
{
    return QString();
}
