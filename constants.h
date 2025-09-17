#ifndef CONSTANTS_H
#define CONSTANTS_H

class BridgeLinxtoCab;
class ClientSocket;
class Server;


#include <QObject>
#include <QMap>


extern QQueue<QByteArray> printQueue;
extern QQueue<QString> makrolineQueue;

class Constants : public QObject
{
    Q_OBJECT
public:
    explicit Constants(QObject *parent = nullptr);
    ~Constants();


//-----------------------------Общие ENUM--------------------------------------------------
    enum class PrinterType{
        Unknown,
        Rynan,
        TSC,
        Docod,
        Cab
    };
    Q_ENUM(PrinterType)

    static const inline QMap<PrinterType, const char*> PrinterTypeText =
    {
        {PrinterType::Unknown, "Не выбран"},
        {PrinterType::Rynan, "Rynan_R20"},
        {PrinterType::TSC, "TSC (Emulator Linx TTO)"},
        {PrinterType::Docod, "Docod TIJ T210"},
        {PrinterType::Cab, "Cab"}
    };

/*
R code;[U:FNC1]0104680127510079210000000000[U:GS]93test
R name;Пивной набор
R batch;Узоры России
R production_datetime;01.09.2025
R expiration_datetime;01.01.2026
R weight;125,2
A 1

код - партия - наименование продукции - дата производства - срок годности - вес
короткие коды 01,21,93
expiration_datetime=||production_datetime=||batch=||code=||name=
*/

    enum class CodeField {
        Field01,
        Field13,
        Field21,
        Field93,
        Field94,
    };
    Q_ENUM(CodeField)

    static const inline QMap<CodeField, QString> CodeFieldNames = {
        {CodeField::Field01, "field01"},
        {CodeField::Field13, "field13"},
        {CodeField::Field21, "field21"},
        {CodeField::Field93, "field93"},
        {CodeField::Field94, "field94"}
    };
    // ~10104935606325621215"a4.h~d02993dGVz
    static inline const QMap<QString, CodeField> fieldMap = {
        {"01", CodeField::Field01},
        {"13", CodeField::Field13},
        {"21", CodeField::Field21},
        {"93", CodeField::Field93},
        {"94", CodeField::Field94}
    };

    static inline const QMap<CodeField, int> fieldLengths = {
        {CodeField::Field01, 14},
        {CodeField::Field21, 13},
        {CodeField::Field93, 4},
        {CodeField::Field13, 0},
        {CodeField::Field94, 0}
    };

    enum class SpinBoxType {
        MakrolineBuffer,
        PrinterBuffer,
        ResponsePrinterBuffer
    };
// ------------------------------Bridge_Linx-----------------------------------------------------
    enum class LinxState {
        Shutdown = 0,
        StartingUp = 1,
        ShuttingDown = 2,
        Running = 3,
        Offline = 4
    };

    enum class TypeLinxCommand
    {
        Unknown = 0,
        Print,              // PRN
        SelectJob,          // SLA
        UpdateJobNamed,     // JDA
        RequestAsyncState,  // EAN
        RequestQueueSize,   // SRC
        RequestState,       // GST
        SetState,           // SST
        AddToBuffer,        // SHD
        StateResponse,      // STS
        PrintComplete,      // PRC
        ErrorResponse,      // ERS
        JobResponse,        // JOB
        GetCurrentError,    // GFT
        GetFreeSpace,       // SFS
        GetJobVars,         // GJF
        GetJobList,         // GJL
        JobListResponse,    // JBL
        UpdateFieldsSerial, // SCF
        FaultResponse,      // FLT
        ClearFaults,        // CAF
        ClearQueue,         // SCB
        SetQueueFields,     // SHO
        AddToQueue          // SDO
    };
    Q_ENUM(TypeLinxCommand)

    static const inline QMap<TypeLinxCommand, const char*> LinxCommandText = {
        {TypeLinxCommand::Unknown, "NAN"}, // Неизвестная команда"
        {TypeLinxCommand::Print, "PRN"}, // Печать
        {TypeLinxCommand::SelectJob, "SLA"}, // Выбор задания"
        {TypeLinxCommand::UpdateJobNamed, "JDA"}, // Обновление полей"
        {TypeLinxCommand::RequestAsyncState, "EAN"}, // Запрос обновления состояния
        {TypeLinxCommand::RequestQueueSize, "SRC"}, // Запрос размера буфера"
        {TypeLinxCommand::RequestState, "GST"}, // Запрос состояния"
        {TypeLinxCommand::SetState, "SST"}, // Смена состояния
        {TypeLinxCommand::AddToBuffer, "SHD"}, // Пополнение очереди"
        {TypeLinxCommand::StateResponse, "STS"}, // Ответ состояния"
        {TypeLinxCommand::PrintComplete, "PRC"},
        {TypeLinxCommand::ErrorResponse, "ERS"}, // Ответ об ошибках""
        {TypeLinxCommand::JobResponse, "JOB"}, // Ответ задания
        {TypeLinxCommand::GetCurrentError, "GFT"}, // Запрос ошибок
        {TypeLinxCommand::GetFreeSpace, "SFS"}, // Запрос свободного места
        {TypeLinxCommand::GetJobVars, "GJF"}, // Запрос списка переменных
        {TypeLinxCommand::GetJobList, "GJL"}, // Запрос списка заданий
        {TypeLinxCommand::JobListResponse, "JBL"},
        {TypeLinxCommand::UpdateFieldsSerial, "SCF"}, // Обновление полей (сериализация)"
        {TypeLinxCommand::FaultResponse, "FLT"}, // Ответ аварии"
        {TypeLinxCommand::ClearFaults, "CAF"}, // Сброс аварий"
        {TypeLinxCommand::ClearQueue, "SCB"}, // "Сброс очереди
        {TypeLinxCommand::SetQueueFields, "SHO"}, // Выбор полей сериализации
        {TypeLinxCommand::AddToQueue, "SDO"} // "Обновление полей сериализации"
    };
// -------------------------------------------Cab----------------------------------------------------------------
    enum class CabState{
        Stop = 0,
        Start = 1,
    };

    enum class StatusPrinterCab : char
    {
        NoError = '-', // Без аварий
        ApplicatorNotUp = 'a',// NoError
        ApplicatorNotDown = 'b', // Аппликатор не внизу
        VacuumPlateEmpty = 'c', // Аппликатор не внизу
        LabelNotDeposit = 'd', // Аппликатор: этикетка не доставлена
        HostError = 'e', // Аппликатор: ошибка хоста
        ReflectiveSensorBlocked = 'f', // Аппликатор: датчик заблокирован
        TampPad90Error = 'g', // Аппликатор: прижим ошибка 90 градусов
        TampPad0Error = 'h', // Аппликатор: прижим ошибка 0 градусов
        TableNotFront = 'i', // Аппликатор: стол не впереди
        TableNotRear = 'j', // Аппликатор: стол не позади
        HeadLiftet = 'k', // Аппликатор: голова поднята
        HeadDown  ='l', // Аппликатор: голова опущена
        ScanresultNegative = 'm', // Ошибка сканирования
        NetworkError = 'n', // Ошибка сети
        NoAirError = 'o', // Нет воздуха
        RFIDError = 'r', // Ошибка RFID метки
        SystemFault = 's', // Авария системы
        USBError = 'u', // Ошибка USB
        ApplicatorError = 'A', // Ошибка аппликатора
        BarcodeDataError = 'B', // Неверные данные ШК или ошибка протокола
        MemoryCardError = 'C', // Ошибка карты памяти
        PrintheadOpen = 'D', // Ошибка карты памяти
        SynchronizationError = 'E', // Печатная голова открыта
        OutOfRibbon = 'F', // Ошибка синхронизации (шаблон не найден)
        PPPReloadRequired = 'G', // Закончилась термотрансферная лента
        HeatingVoltageProblem = 'H', // Необходима перезагрузка PPP
        CutterJammed = 'M', // Обрезчик: замятие
        LabelMaterialTooThick = 'N', // Обрезчик: материал слишком тонкий
        OutOfMemory = 'O', // Недостаточно памяти
        OutOfPaper = 'P', // Закончилась бумага
        RibbonInThermalDirectMode = 'R', // В режиме прямого нанесения обнаружена термотрансферная лента
        RibonsaverMalfunction = 'S', // Ошибка функции сохранения термотрансферной ленты
        InputBufferOverflow = 'V', // Входящий буфер переполнен
        PrintHeadOverheated = 'W', // Перегрев печатной головы
        ExternalIOError = 'X', // Активирована внешняя ошибка
        PrintHeadError = 'Y', // Ошибка печатной головы
        PrintheadDamaged = 'Z' // Печатная голова повреждена
    };

    enum class TypeCommandCab{
        AddCode,
        ClearBuffers,
        StartPrint,
        StopPrint,
        RequestStatus,
    };

    static const inline QMap<TypeCommandCab, QByteArray> TypeCommandCabText =
    {
        {TypeCommandCab::StartPrint, QByteArray::fromHex("1B0200111B03B4")},
        {TypeCommandCab::StopPrint,  QByteArray::fromHex("1B0200121B03B3")},
        {TypeCommandCab::RequestStatus,  QByteArray::fromHex("1B0200141B03B1")},
        {TypeCommandCab::ClearBuffers, QByteArray("CLEAR")},
    };
    Q_ENUM(TypeCommandCab)

    // Константы
    static inline uint batchCount = 0; // Количество продуктов в текущей сессии
    static inline uint totalCount = 0; // Общее количество продуктов
    static inline uint countPrinted{0};// Счётчик напечатанных этикеток принтером
    static inline uint countBufferInPrinter = 0; // Сколько в буфере принтера
    static inline uint idLabel{0}; // ID текущей этикетки
    static inline bool AutoAndManualModes = false; // Режим работы (авто/ручной)
    static inline uint LENGTH_CODE;
    static inline int maxReconnectAttempts = 100;
    static inline int reconnectInterval = 5000;
    static inline QString lastWeight = "";
    static inline QString labelTemplate; // Переменная содержащая этикетку
    static inline QMap<QString, QString> variables; // Разбивка полей по строкам parts
    static inline PrinterType currentPrinter = PrinterType::Unknown;
    static inline CabState currentStatusCab = CabState::Stop;
    static inline LinxState currentStatusMakroline = LinxState::Shutdown;
};

#endif // CONSTANTS_H
