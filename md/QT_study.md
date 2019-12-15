# QT
## Signal and slots
- http://doc.qt.io/archives/qt-4.8/signalsandslots.html
- Callbacks have two fundamental flaws: Firstly, they are not type-safe. We can never be certain that the processing function will call the callback with the correct arguments. Secondly, the callback is strongly coupled to the processing function since the processing function must know which callback to call.
  - So, Qt's signals and slots mechanism ensures that if you connect a signal to a slot, the slot will be called with the signal's parameters at the right time. Signals and slots can take any number of arguments of any type. They are completely type safe.
- Signal은 signal을 보낸는 것이고, slot은 그때 수행하는 함수이다.  signal 함수는 void로 return되어져야 하고 , emit 을 가지고 SIGNLAL을 발생시킨다. connect라는 것으로 object->SIGNAL object->SLOT을 연결하여 해당 signal이 발생하면 해당 slot을 불러서 처리하게 된다. 보낸 시그널이 해당 object에 signal로 정의되어져있지 않으면(connect로 연결되어져있지 않으면) ,  mapping되는 slot이 없으므로 무시 된다. 

```cpp
#include <QObject>

class Counter : public QObject
{
    Q_OBJECT

public:
    Counter() { m_value = 0; }

    int value() const { return m_value; }

public slots:
    void setValue(int value);

signals:
    void valueChanged(int newValue);

private:
    int m_value;
};

void Counter::setValue(int value)
{
    if (value != m_value) {
        m_value = value;
        emit valueChanged(value);
    }
}

    Counter a, b;
    QObject::connect(&a, SIGNAL(valueChanged(int)),
                     &b, SLOT(setValue(int)));

    a.setValue(12);     // a.value() == 12, b.value() == 12
    b.setValue(48);     // a.value() == 12, b.value() == 48
```

### Slots
- You can also define slots to be virtual, which we have found quite useful in practice
- 
