#include <QObject>
#include <QModbusRtuSerialMaster>
#include <QModbusDataUnit>

class ModbusWorker : public QObject
{
    Q_OBJECT
public:
    explicit ModbusWorker(const QString &portName, QObject *parent = nullptr);
    ~ModbusWorker();

public slots:
    void readRegister(int startAddress, int numberOfEntries, int serverAddress);
    void onReplyFinished();

signals:
    void dataRead(QModbusDataUnit result);
    void errorOccurred(QString error);

private:
    QModbusRtuSerialMaster *modbusDevice;
    QString portName;

    bool connectToDevice();
};
